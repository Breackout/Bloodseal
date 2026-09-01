#include "Entity.hpp"
#include "Render.hpp"
#include <cmath>

// ==================== Camera ====================

void Camera::SetBounds(float worldW, float worldH)
{
    worldWidth = worldW;
    worldHeight = worldH;
}

void Camera::Update(const vec2D& targetPos, float targetW, float targetH, float dt)
{
    float visibleWidth  = ScreenWidth  / zoom;
    float visibleHeight = ScreenHeight / zoom;

    float desiredX = (targetPos.x + targetW / 2.0f) - visibleWidth / 2.0f;
    float desiredY = (targetPos.y + targetH / 2.0f) - visibleHeight / 2.0f;

    float t = 1.0f - std::exp(-smoothSpeed * dt);
    pos.x += (desiredX - pos.x) * t;
    pos.y += (desiredY - pos.y) * t;

    if (worldWidth > 0.0f && worldHeight > 0.0f)
    {
        if (worldWidth <= visibleWidth)
            pos.x = (worldWidth - visibleWidth) * 0.5f; // mondo più piccolo della vista -> centra
        else
        {
            if (pos.x < 0.0f) pos.x = 0.0f;
            if (pos.x > worldWidth - visibleWidth) pos.x = worldWidth - visibleWidth;
        }

        if (worldHeight <= visibleHeight)
            pos.y = (worldHeight - visibleHeight) * 0.5f;
        else
        {
            if (pos.y < 0.0f) pos.y = 0.0f;
            if (pos.y > worldHeight - visibleHeight) pos.y = worldHeight - visibleHeight;
        }
    }
}


// ==================== Player ====================

void Player::move(float dt, const bool* keys)
{
    moving = false;

    if (keys[SDL_SCANCODE_A])
    {
        dir = -1;
        vel.x -= acceleration * dt;
        moving = true;
    }
    if (keys[SDL_SCANCODE_D])
    {
        dir = 1;
        vel.x += acceleration * dt;
        moving = true;
    }
    if(isGround)
    {
        if (keys[SDL_SCANCODE_SPACE])
        {
            vel.y = jumpForce;
            isGround = false;
        }
    }

    // --- DASH ---
    // avvia il dash solo se il tasto è appena stato premuto (edge) e non sei in cooldown
    if (keys[SDL_SCANCODE_LSHIFT] && !dashKeyWasPressed && dashCooldownTimer <= 0.0f)
    {
        isDashing = true;
        dashTimer = dashDuration;        // es. 0.15f, quanto dura il "colpo"
        vel.x = dashSpeed * dir;         // impulso secco, non accumulo
    }
    dashKeyWasPressed = keys[SDL_SCANCODE_LSHIFT];

    if (isDashing)
    {
        dashTimer -= dt;
        if (dashTimer <= 0.0f)
        {
            isDashing = false;
            dashCooldownTimer = dashCooldownDuration; // es. 2.0f, ora sì è un vero cooldown
        }
    }
    if (dashCooldownTimer > 0.0f)
        dashCooldownTimer -= dt;

    // frizione
    if (!moving && !isDashing)
        vel.x *= 0.9f;

    // clamp, saltato solo mentre il dash è attivo
    if (!isDashing)
    {
        if (vel.x > maxSpeed) vel.x = maxSpeed;
        if (vel.x < -maxSpeed) vel.x = -maxSpeed;
    }

    vel.y += GRAVITY * dt;
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
}


void Player::Update(float dt, const bool* keys)
{
    move(dt, keys);
}

void Player::Draw(const Camera& camera)
{
    // playerBox qui è puramente per il disegno: coordinate relative alla camera
    playerBox.x = pos.x - camera.pos.x;
    playerBox.y = pos.y - camera.pos.y;
    playerBox.w = width;
    playerBox.h = height;

    SDL_SetRenderDrawColor(rend.GetRenderer(), 0, 0, 0, 255);
    SDL_RenderFillRect(rend.GetRenderer(), &playerBox);
}
