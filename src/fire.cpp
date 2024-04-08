#include <math.h>
#include "fire.h"

void initFire(Particle m_fire[], int size, const glm::vec3& minBounds, const glm::vec3& maxBounds) {
    for (int i = 0; i < size; ++i) {
        m_fire[i].active = true;
        m_fire[i].life = 1.0f;
        m_fire[i].fade = float(rand() % 100) / 1000.0f + 0.003f;
        m_fire[i].color = WHITE; // init color white
        // init random speed at all dir
        m_fire[i].speed.x = float((rand() % 50) - 25.0f) * 0.1f;
        m_fire[i].speed.y = float((rand() % 50) - 25.0f) * 0.1f;
        m_fire[i].speed.z = float((rand() % 50) - 25.0f) * 0.1f;
        m_fire[i].acceleration = glm::vec3(0.0f, 0.5f, 0.0f);
        // init pos: random but denser at bottom
        // todo: change this
        float heightBias = 1.0f - (float(i) / float(size));
        glm::vec3 biasedMinBounds = minBounds;
        glm::vec3 biasedMaxBounds = maxBounds;
        biasedMinBounds.y = minBounds.y;
        biasedMaxBounds.y -= heightBias * (maxBounds.y - minBounds.y) * 0.5f;
        m_fire[i].pos.x = float(rand()) / RAND_MAX * (biasedMaxBounds.x - biasedMinBounds.x) + biasedMinBounds.x;
        m_fire[i].pos.y = float(rand()) / RAND_MAX * (biasedMaxBounds.y - biasedMinBounds.y) + biasedMinBounds.y;
        m_fire[i].pos.z = float(rand()) / RAND_MAX * (biasedMaxBounds.z - biasedMinBounds.z) + biasedMinBounds.z;
    }
}

void updateFire(Particle m_fire[], int size, const glm::vec3& minBounds, const glm::vec3& maxBounds) {
    for (int i = 0; i < size; ++i) {
        if (m_fire[i].active) {
            // draw in main app
            // move pos for next loop
            // TODO: check still in bounds, if not set dead?
            m_fire[i].pos.x += m_fire[i].speed.x / (375);
            m_fire[i].pos.y += m_fire[i].speed.y / (500);
            m_fire[i].pos.z += m_fire[i].speed.z / (375);
            glm::vec3 baseFire = glm::vec3(minBounds.x + (maxBounds.x - minBounds.x) / 2, minBounds.y, minBounds.z + (maxBounds.z - minBounds.z) / 2);
            if ((m_fire[i].pos.x > baseFire.x) && (m_fire[i].pos.y > (0.1 + baseFire.y))) {
                m_fire[i].acceleration.x = -0.3f;
            }
            else if ((m_fire[i].pos.x < baseFire.x) && (m_fire[i].pos.y > (0.1 + baseFire.y))) {
                m_fire[i].acceleration.x = 0.3f;
            }
            else {
                m_fire[i].acceleration.x = 0.0f;
            }
            // TODO: limit shape of the fire
            if (m_fire[i].pos.y > 2 * maxBounds.y) m_fire[i].life = 0.0f;
            m_fire[i].speed += m_fire[i].acceleration;
            m_fire[i].life -= m_fire[i].fade;
            if (m_fire[i].life < 0.0f) {
                m_fire[i].life = 1.0f;					// Give It New Life
                m_fire[i].fade = float(rand() % 100) / 1000.0f + 0.003f;	// Random Fade Value
                m_fire[i].pos = baseFire;					// Center On Z Axis
                m_fire[i].color = WHITE;
                m_fire[i].speed.x = float((rand() % 50) - 25.0f) * 0.1f;
                m_fire[i].speed.y = float((rand() % 50) - 25.0f) * 0.1f;
                m_fire[i].speed.z = float((rand() % 50) - 25.0f) * 0.1f;
            }
            // change color according to lifespan left
            else if (m_fire[i].life < 0.4f)
            {
                m_fire[i].color = RED;
            }
            else if (m_fire[i].life < 0.6f)
            {
                m_fire[i].color = ORANGE;
            }
            else if (m_fire[i].life < 0.75f)
            {
                m_fire[i].color = YELLOW;
            }
            else if (m_fire[i].life < 0.85f)
            {
                m_fire[i].color = BLUE;
            }
        }
    }
}