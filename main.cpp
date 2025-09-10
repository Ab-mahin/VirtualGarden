#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <vector>

const int MAX_PARTICLES = 3500;  // Rain particles
const int NUM_CLOUDS = 10;     // Number of clouds
const int PUFFS_PER_CLOUD = 6; // Reduced for a simpler cloud shape
const int RIPPLES = 10;       // Number of pond ripples

struct Particle {
    float x, y, z;
    float speed;
};

struct CloudPuff {
    float x, y, z;
    float size;
    float alpha;
};

struct Cloud {
    float x, y, z;
    float speed;
    float width;
    CloudPuff puffs[PUFFS_PER_CLOUD];
};

struct Ripple {
    float x, z;
    float radius;
    float maxRadius;
    float speed;
    float alpha;
};

struct Sphere {
    float x, y, z;
    float radius;
    float speed;
};

Particle particles[MAX_PARTICLES];
Cloud clouds[NUM_CLOUDS];
Ripple ripples[RIPPLES];
Sphere sphere = { -2.0f, 0.0f, -2.0f, 0.3f, 0.1f }; // Sphere starts at (-2, 0, -2)
bool doorOpen = false;
const float houseX = 0.0f, houseY = 0.75f, houseZ = 0.0f; // House position
const float pondX = 3.0f, pondY = 0.01f, pondZ = 3.0f; // Pond position
float cameraDistance = 15.0f; // Distance from camera to center
float cameraAngleX = 45.0f;  // Horizontal angle (degrees)
float cameraAngleY = 18.0f;  // Vertical angle (degrees)
const float cameraSpeed = 0.1f; // Camera movement speed
const float angleSpeed = 2.0f;  // Camera rotation speed (degrees)

void initParticles() {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        particles[i].x = ((rand() % 200) - 100) / 50.0f;
        particles[i].y = (rand() % 100) / 10.0f + 2.0f;
        particles[i].z = ((rand() % 200) - 100) / 50.0f;
        particles[i].speed = 0.02f + (rand() % 10) / 500.0f;
    }
}

void initClouds() {
    clouds[0].x = -5.0f;
    clouds[0].y = 4.0f;
    clouds[0].z = 2.0f;
    clouds[0].speed = 0.002f;
    clouds[0].width = 2.0f;
    
    clouds[1].x = -4.0f;
    clouds[1].y = 7.0f;
    clouds[1].z = -3.0f;
    clouds[1].speed = 0.001f;
    clouds[1].width = 1.5f;
    
    for (int i = 2; i < NUM_CLOUDS; ++i) {
        clouds[i].x = -15.0f + (rand() % 300) / 10.0f;
        clouds[i].y = 4.0f + (rand() % 40) / 10.0f;
        clouds[i].z = -15.0f + (rand() % 300) / 10.0f;
        clouds[i].speed = (rand() % 8) / 2000.0f;
        clouds[i].width = 1.5f + (rand() % 10) / 10.0f;
    }
    
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        for (int j = 0; j < PUFFS_PER_CLOUD; ++j) {
            clouds[i].puffs[j].x = (rand() % 100 - 50) / 100.0f * clouds[i].width * 0.5f;
            clouds[i].puffs[j].y = (rand() % 50 - 25) / 100.0f * 0.5f;
            clouds[i].puffs[j].z = (rand() % 100 - 50) / 100.0f * clouds[i].width * 0.5f;
            clouds[i].puffs[j].size = 0.5f + (rand() % 10) / 20.0f;
            clouds[i].puffs[j].alpha = 0.8f + (rand() % 20) / 100.0f;
        }
    }
}

void initRipples() {
    for (int i = 0; i < RIPPLES; ++i) {
        ripples[i].x = ((rand() % 100) - 50) / 100.0f * 1.5f;
        ripples[i].z = ((rand() % 100) - 50) / 100.0f * 1.5f;
        ripples[i].radius = 0.1f * (rand() % 10) / 10.0f;
        ripples[i].maxRadius = 0.5f + (rand() % 100) / 200.0f;
        ripples[i].speed = 0.01f + (rand() % 5) / 500.0f;
        ripples[i].alpha = 0.8f;
    }

void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        particles[i].y -= particles[i].speed;
        if (particles[i].y < 0.0f) {
            particles[i].y = 5.0f;
            particles[i].x = ((rand() % 600) - 200) / 50.0f;
            particles[i].z = ((rand() % 600) - 200) / 50.0f;
        }
    }
}

void updateClouds() {
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        clouds[i].x += clouds[i].speed;
        if (clouds[i].x > 15.0f) {
            clouds[i].x = -15.0f;
            clouds[i].y = 8.0f + (rand() % 40) / 10.0f;
            clouds[i].z = ((rand() % 500) - 250) / 50.0f;
        }
    }
}

void updateRipples() {
    for (int i = 0; i < RIPPLES; ++i) {
        ripples[i].radius += ripples[i].speed;
        ripples[i].alpha = 0.8f * (1.0f - ripples[i].radius / ripples[i].maxRadius);
        if (ripples[i].radius >= ripples[i].maxRadius || ripples[i].alpha <= 0.1f) {
            ripples[i].radius = 0.0f;
            ripples[i].x = ((rand() % 100) - 50) / 100.0f * 1.5f;
            ripples[i].z = ((rand() % 100) - 50) / 100.0f * 1.5f;
            ripples[i].alpha = 0.8f;
        }
    }
}

bool isValidSpherePosition(float newX, float newZ) {
    if (newX >= -1.0f - sphere.radius && newX <= 1.0f + sphere.radius &&
        newZ >= -1.0f - sphere.radius && newZ <= 1.0f + sphere.radius) {
        return false;
    }
    float dx = newX - pondX;
    float dz = newZ - pondZ;
    if (sqrt(dx * dx + dz * dz) <= 2.0f + sphere.radius) {
        return false;
    }
    return true;
}

void drawParticles() {
    glColor3f(0.7f, 0.7f, 1.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        float rainLength = 0.1f;
        glVertex3f(particles[i].x, particles[i].y, particles[i].z);
        glVertex3f(particles[i].x + rainLength, particles[i].y - rainLength * 0.5f, particles[i].z);
    }
    glEnd();
}

void drawSphere(float radius, int slices, int stacks) {
    for (int i = 0; i < stacks; ++i) {
        float phi1 = M_PI * (float)i / stacks - M_PI / 2.0f;
        float phi2 = M_PI * (float)(i + 1) / stacks - M_PI / 2.0f;
        
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * M_PI * (float)j / slices;
            
            float x1 = radius * cos(phi1) * cos(theta);
            float y1 = radius * sin(phi1);
            float z1 = radius * cos(phi1) * sin(theta);
            
            float x2 = radius * cos(phi2) * cos(theta);
            float y2 = radius * sin(phi2);
            float z2 = radius * cos(phi2) * sin(theta);
            
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}

void drawClouds() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        glPushMatrix();
        glTranslatef(clouds[i].x, clouds[i].y, clouds[i].z);
        
        for (int j = 0; j < PUFFS_PER_CLOUD; ++j) {
            CloudPuff* puff = &clouds[i].puffs[j];
            glColor4f(1.0f, 1.0f, 1.0f, puff->alpha);
            glPushMatrix();
            glTranslatef(puff->x, puff->y, puff->z);
            drawSphere(puff->size, 12, 12);
            glPopMatrix();
        }
        
        glPopMatrix();
    }
    glDisable(GL_BLEND);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_O) {
            doorOpen = true;
            std::cout << "O key pressed: Door set to OPEN" << std::endl;
        } else if (key == GLFW_KEY_C) {
            doorOpen = false;
            std::cout << "C key pressed: Door set to CLOSED" << std::endl;
        } else if (key == GLFW_KEY_W) {
            cameraDistance = std::max(2.0f, cameraDistance - cameraSpeed);
            std::cout << "W key pressed: Zoom in" << std::endl;
        } else if (key == GLFW_KEY_S) {
            cameraDistance = std::min(20.0f, cameraDistance + cameraSpeed);
            std::cout << "S key pressed: Zoom out" << std::endl;
        } else if (key == GLFW_KEY_A) {
            cameraAngleX += angleSpeed;
            std::cout << "A key pressed: Rotate left" << std::endl;
        } else if (key == GLFW_KEY_D) {
            cameraAngleX -= angleSpeed;
            std::cout << "D key pressed: Rotate right" << std::endl;
        } else if (key == GLFW_KEY_Q) {
            cameraAngleY = std::min(80.0f, cameraAngleY + angleSpeed);
            std::cout << "Q key pressed: Tilt up" << std::endl;
        } else if (key == GLFW_KEY_E) {
            cameraAngleY = std::max(10.0f, cameraAngleY - angleSpeed);
            std::cout << "E key pressed: Tilt down" << std::endl;
        } else if (key == GLFW_KEY_LEFT) {
            float newX = sphere.x - sphere.speed;
            if (isValidSpherePosition(newX, sphere.z)) {
                sphere.x = newX;
                std::cout << "Left key pressed: Sphere moved to (" << sphere.x << ", " << sphere.z << ")" << std::endl;
            }
        } else if (key == GLFW_KEY_RIGHT) {
            float newX = sphere.x + sphere.speed;
            if (isValidSpherePosition(newX, sphere.z)) {
                sphere.x = newX;
                std::cout << "Right key pressed: Sphere moved to (" << sphere.x << ", " << sphere.z << ")" << std::endl;
            }
        } else if (key == GLFW_KEY_UP) {
            float newZ = sphere.z - sphere.speed;
            if (isValidSpherePosition(sphere.x, newZ)) {
                sphere.z = newZ;
                std::cout << "Up key pressed: Sphere moved to (" << sphere.x << ", " << sphere.z << ")" << std::endl;
            }
        } else if (key == GLFW_KEY_DOWN) {
            float newZ = sphere.z + sphere.speed;
            if (isValidSpherePosition(sphere.x, newZ)) {
                sphere.z = newZ;
                std::cout << "Down key pressed: Sphere moved to (" << sphere.x << ", " << sphere.z << ")" << std::endl;
            }
        }
    }
}

void drawCube(float x, float y, float z, float w, float h, float d) {
    float hw = w / 2.0f, hh = h / 2.0f, hd = d / 2.0f;

    glPushMatrix();
    glTranslatef(x, y, z);
    glBegin(GL_QUADS);

    glVertex3f(-hw, -hh, hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(-hw, hh, hd);

    glVertex3f(-hw, -hh, -hd);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(-hw, hh, -hd);

    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, -hh, hd);
    glVertex3f(-hw, hh, hd);
    glVertex3f(-hw, hh, -hd);

    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(hw, hh, -hd);

    glVertex3f(-hw, hh, -hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(-hw, hh, hd);

    glVertex3f(-hw, -hh, -hd);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(-hw, -hh, hd);

    glEnd();
    glPopMatrix();
}

void drawCylinder(float baseRadius, float topRadius, float height, int slices) {
    float angle, x1, y1, x2, y2;
    float sliceAngle = 2.0f * M_PI / slices;
    
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= slices; ++i) {
        angle = i * sliceAngle;
        x1 = cos(angle) * baseRadius;
        y1 = sin(angle) * baseRadius;
        x2 = cos(angle) * topRadius;
        y2 = sin(angle) * topRadius;
        glVertex3f(x1, 0.0f, y1);
        glVertex3f(x2, height, y2);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, height, 0.0f);
    for (int i = 0; i <= slices; ++i) {
        angle = i * sliceAngle;
        x1 = cos(angle) * topRadius;
        y1 = sin(angle) * topRadius;
        glVertex3f(x1, height, y1);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = slices; i >= 0; --i) {
        angle = i * sliceAngle;
        x1 = cos(angle) * baseRadius;
        y1 = sin(angle) * baseRadius;
        glVertex3f(x1, 0.0f, y1);
    }
    glEnd();
}

void drawTree(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    
    glColor3f(0.5f, 0.3f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.0f);
    drawCylinder(0.15f, 0.1f, 1.0f, 12);
    glPopMatrix();
    
    glColor3f(0.1f, 0.5f, 0.1f);
    
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    drawSphere(0.5f, 12, 12);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.0f, 1.8f, 0.0f);
    drawSphere(0.4f, 12, 12);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.0f, 2.1f, 0.0f);
    drawSphere(0.3f, 12, 12);
    glPopMatrix();
    
    glPopMatrix();
}

void drawHouse() {
    glPushMatrix();
    glTranslatef(houseX, houseY, houseZ);

    glColor3f(0.8f, 0.8f, 0.8f);
    drawCube(0, 0.0f, 0, 2, 1.5, 2);

    glColor3f(0.6f, 0.3f, 0.1f);
    
    glBegin(GL_TRIANGLES);
    glVertex3f(-1.0f, 0.75f, 1.0f);
    glVertex3f(1.0f, 0.75f, 1.0f);
    glVertex3f(0.0f, 1.75f, 1.0f);
    glEnd();
    
    glBegin(GL_TRIANGLES);
    glVertex3f(-1.0f, 0.75f, -1.0f);
    glVertex3f(1.0f, 0.75f, -1.0f);
    glVertex3f(0.0f, 1.75f, -1.0f);
    glEnd();

    glBegin(GL_QUADS);
    glVertex3f(-1.0f, 0.75f, 1.0f);
    glVertex3f(-1.0f, 0.75f, -1.0f);
    glVertex3f(0.0f, 1.75f, -1.0f);
    glVertex3f(0.0f, 1.75f, 1.0f);

    glVertex3f(1.0f, 0.75f, 1.0f);
    glVertex3f(1.0f, 0.75f, -1.0f);
    glVertex3f(0.0f, 1.75f, -1.0f);
    glVertex3f(0.0f, 1.75f, 1.0f);
    glEnd();

    glColor3f(0.4f, 0.2f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 1.01f);
    if (doorOpen)
        glRotatef(-90, 0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.25f, 0.0f, 0.0f);
    glVertex3f(0.25f, 0.0f, 0.0f);
    glVertex3f(0.25f, 0.75f, 0.0f);
    glVertex3f(-0.25f, 0.75f, 0.0f);
    glEnd();
    glPopMatrix();

    glColor3f(0.9f, 0.9f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.6f, 0.0f, 1.01f);
    glBegin(GL_QUADS);
    glVertex3f(-0.25f, -0.25f, 0.0f);
    glVertex3f(0.25f, -0.25f, 0.0f);
    glVertex3f(0.25f, 0.25f, 0.0f);
    glVertex3f(-0.25f, 0.25f, 0.0f);
    glEnd();
    
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(-0.25f, 0.0f, 0.01f);
    glVertex3f(0.25f, 0.0f, 0.01f);
    glVertex3f(0.0f, -0.25f, 0.01f);
    glVertex3f(0.0f, 0.25f, 0.01f);
    glEnd();
    glPopMatrix();
    
    glColor3f(0.9f, 0.9f, 1.0f);
    glPushMatrix();
    glTranslatef(0.6f, 0.0f, 1.01f);
    glBegin(GL_QUADS);
    glVertex3f(-0.25f, -0.25f, 0.0f);
    glVertex3f(0.25f, -0.25f, 0.0f);
    glVertex3f(0.25f, 0.25f, 0.0f);
    glVertex3f(-0.25f, 0.25f, 0.0f);
    glEnd();
    
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(-0.25f, 0.0f, 0.01f);
    glVertex3f(0.25f, 0.0f, 0.01f);
    glVertex3f(0.0f, -0.25f, 0.01f);
    glVertex3f(0.0f, 0.25f, 0.01f);
    glEnd();
    glPopMatrix();

    glColor3f(0.5f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(0.6f, 1.2f, -0.3f);
    drawCube(0.0f, 0.0f, 0.0f, 0.3f, 0.6f, 0.3f);
    glPopMatrix();

    drawTree(2.5f, 0.0f, -0.5f);

    glPopMatrix();
}

void drawPond() {
    glPushMatrix();
    glTranslatef(pondX, pondY, pondZ);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(0.2f, 0.4f, 0.8f, 0.8f);
    
    float pondRadius = 2.0f;
    int segments = 36;
    float angle, x, z;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        angle = i * 2.0f * M_PI / segments;
        x = cos(angle) * pondRadius;
        z = sin(angle) * pondRadius;
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glColor3f(0.6f, 0.5f, 0.3f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        angle = i * 2.0f * M_PI / segments;
        x = cos(angle) * pondRadius;
        z = sin(angle) * pondRadius;
        glVertex3f(x * 1.1f, 0.05f, z * 1.1f);
        glVertex3f(x, 0.01f, z);
    }
    glEnd();
    
    updateRipples();
    
    for (int i = 0; i < RIPPLES; ++i) {
        Ripple* ripple = &ripples[i];
        if (ripple->radius > 0.01f) {
            glColor4f(1.0f, 1.0f, 1.0f, ripple->alpha * 0.3f);
            glBegin(GL_LINE_LOOP);
            for (int j = 0; j < segments; ++j) {
                angle = j * 2.0f * M_PI / segments;
                x = ripple->x + cos(angle) * ripple->radius;
                z = ripple->z + sin(angle) * ripple->radius;
                glVertex3f(x, 0.02f, z);
            }
            glEnd();
        }
    }
    
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawSky() {
    glColor3f(0.4f, 0.6f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    
    float radius = 50.0f;
    int segments = 20;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, radius, 0.0f);
    for (int i = 0; i <= segments * 2; i++) {
        float angle = i * M_PI / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glPopMatrix();
}

void drawGround() {
    glColor3f(0.3f, 0.6f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-50.0f, 0.0f, -50.0f);
    glVertex3f(-50.0f, 0.0f, 50.0f);
    glVertex3f(50.0f, 0.0f, 50.0f);
    glVertex3f(50.0f, 0.0f, -50.0f);
    glEnd();
    
    glColor3f(0.2f, 0.5f, 0.2f);
    glBegin(GL_LINES);
    for (float x = -10.0f; x <= 10.0f; x += 1.0f) {
        for (float z = -10.0f; z <= 10.0f; z += 1.0f) {
            if (x > -1.5f && x < 1.5f && z > -1.5f && z < 1.5f)
                continue;
            float dx = x - pondX;
            float dz = z - pondZ;
            float distToPond = sqrt(dx*dx + dz*dz);
            if (distToPond < 2.2f)
                continue;
            glVertex3f(x - 0.1f, 0.01f, z - 0.1f);
            glVertex3f(x + 0.1f, 0.01f, z + 0.1f);
            glVertex3f(x - 0.1f, 0.01f, z + 0.1f);
            glVertex3f(x + 0.1f, 0.01f, z - 0.1f);
        }
    }
    glEnd();
}

void drawControllableSphere() {
    glColor3f(1.0f, 0.5f, 0.0f);
    glPushMatrix();
    glTranslatef(sphere.x, sphere.y + sphere.radius, sphere.z);
    drawSphere(sphere.radius, 12, 12);
    glPopMatrix();
}

int main() {
    srand(time(0));
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Scene with House, Tree, and Sphere", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(8.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = 800.0f / 600.0f;
    float fov = 45.0f, near = 0.1f, far = 100.0f;
    float top = near * tan(fov * 3.14159f / 360.0f);
    float right = top * aspect;
    glFrustum(-right, right, -top, top, near, far);

    initParticles();
    initClouds();
    initRipples();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float radX = cameraAngleX * M_PI / 180.0f;
        float radY = cameraAngleY * M_PI / 180.0f;
        float eyeX = houseX + cameraDistance * cos(radY) * sin(radX);
        float eyeY = houseY + cameraDistance * sin(radY);
        float eyeZ = houseZ + cameraDistance * cos(radY) * cos(radX);
        float centerX = houseX, centerY = houseY, centerZ = houseZ;
        float upX = 0.0f, upY = 1.0f, upZ = 0.0f;

        float fX = centerX - eyeX, fY = centerY - eyeY, fZ = centerZ - eyeZ;
        float fLen = sqrt(fX * fX + fY * fY + fZ * fZ);
        fX /= fLen; fY /= fLen; fZ /= fLen;

        float sX = upY * fZ - upZ * fY;
        float sY = upZ * fX - upX * fZ;
        float sZ = upX * fY - upY * fX;
        float sLen = sqrt(sX * sX + sY * sY + sZ * sZ);
        sX /= sLen; sY /= sLen; sZ /= sLen;

        float uX = fY * sZ - fZ * sY;
        float uY = fZ * sX - fX * sZ;
        float uZ = fX * sY - fY * sX;

        float m[16] = {
            sX, uX, -fX, 0,
            sY, uY, -fY, 0,
            sZ, uZ, -fZ, 0,
            -(sX * eyeX + sY * eyeY + sZ * eyeZ),
            -(uX * eyeX + uY * eyeY + uZ * eyeZ),
            (fX * eyeX + fY * eyeY + fZ * eyeZ), 1
        };
        glMultMatrixf(m);

        glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

        drawSky();
        drawGround();
        updateClouds();
        drawClouds();
        drawHouse();
        drawPond();
        updateParticles();
        drawParticles();
        drawControllableSphere();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}