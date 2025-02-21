//
// Created by Admin on 21/02/2025.
//

#include "camera.h"

using namespace My;

Camera::Camera(pointf3 position, vecf3 up, float yaw, float pitch)
    : Front(vecf3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM) {
  Position = position;
  WorldUp = up;
  Yaw = yaw;
  Pitch = pitch;
  updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY,
               float upZ, float yaw, float pitch)
    : Front(vecf3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM) {
  Position = pointf3(posX, posY, posZ);
  WorldUp = vecf3(upX, upY, upZ);
  Yaw = yaw;
  Pitch = pitch;
  updateCameraVectors();
}

transformf Camera::GetViewMatrix() const {
  return transformf::look_at(Position, Position + Front, WorldUp);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
  float velocity = MovementSpeed * deltaTime;
  if (direction == FORWARD)
    Position += Front * velocity;
  if (direction == BACKWARD)
    Position -= Front * velocity;
  if (direction == LEFT)
    Position -= Right * velocity;
  if (direction == RIGHT)
    Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset,
                                  GLboolean constrainPitch) {
  xoffset *= MouseSensitivity;
  yoffset *= MouseSensitivity;

  Yaw += xoffset;
  Pitch += yoffset;

  // Make sure that when pitch is out of bounds, screen doesn't get flipped
  if (constrainPitch) {
    if (Pitch > 89.0f)
      Pitch = 89.0f;
    if (Pitch < -89.0f)
      Pitch = -89.0f;
  }

  // Update Front, Right and Up Vectors using the updated Euler angles
  updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
  if (Zoom >= 1.0f && Zoom <= 45.0f)
    Zoom -= yoffset;
  if (Zoom <= 1.0f)
    Zoom = 1.0f;
  if (Zoom >= 45.0f)
    Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
  // Calculate the new Front vector
  vecf3 front;
  front[0] = cos(to_radian(Yaw)) * cos(to_radian(Pitch));
  front[1] = sin(to_radian(Pitch));
  front[2] = sin(to_radian(Yaw)) * cos(to_radian(Pitch));
  Front = front.normalize();
  // Also re-calculate the Right and Up vector
  Right =
      Front.cross(WorldUp)
          .normalize();  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
  Up = Right.cross(Front).normalize();
}
