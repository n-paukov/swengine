#include "precompiled.h"

#pragma hdrstop

#include "geometry.h"

#include <glm/geometric.hpp>
#include "MathUtils.h"

Plane::Plane()
{

}

Plane::Plane(const glm::vec3& normal, float distance)
  : m_normal(normal), m_distance(distance)
{

}

void Plane::setNormal(const glm::vec3& normal)
{
  m_normal = normal;
}

glm::vec3 Plane::getNormal() const
{
  return m_normal;
}

void Plane::setDistance(float distance)
{
  m_distance = distance;
}

float Plane::getDistance() const
{
  return m_distance;
}

void Plane::normalize()
{
  float inv_length = 1.0f / glm::length(m_normal);

  m_normal *= inv_length;
  m_distance *= inv_length;
}

Plane Plane::fromUnnormalized(const glm::vec3& normal, float distance)
{
  Plane plane(normal, distance);
  plane.normalize();

  return plane;
}

Plane Plane::getInverse() const
{
  return Plane(-m_normal, -m_distance);
}

Frustum::Frustum()
{

}

Frustum::Frustum(const std::array<Plane, 6>& planes)
  : m_planes(planes)
{

}

const Plane& Frustum::getPlane(size_t index) const
{
  return m_planes[index];
}

Plane& Frustum::getPlane(size_t index)
{
  return m_planes[index];
}

const Plane& Frustum::getPlane(FrustumPlane plane) const
{
  return m_planes[static_cast<size_t>(plane)];
}

Plane& Frustum::getPlane(FrustumPlane plane)
{
  return m_planes[static_cast<size_t>(plane)];
}

void Frustum::setPlane(size_t index, const Plane& plane)
{
  m_planes[index] = plane;
}

void Frustum::setPlane(FrustumPlane planeType, const Plane& plane)
{
  m_planes[static_cast<size_t>(planeType)] = plane;
}

std::array<glm::vec3, 8> Frustum::getCorners() const
{
  return {
    GeometryUtils::getPlanesIntersection(getPlane(FrustumPlane::Near), getPlane(FrustumPlane::Left), getPlane(FrustumPlane::Bottom)),
    GeometryUtils::getPlanesIntersection(getPlane(FrustumPlane::Near), getPlane(FrustumPlane::Right), getPlane(FrustumPlane::Bottom)),
    GeometryUtils::getPlanesIntersection(getPlane(FrustumPlane::Near), getPlane(FrustumPlane::Right), getPlane(FrustumPlane::Top)),
    GeometryUtils::getPlanesIntersection(getPlane(FrustumPlane::Near), getPlane(FrustumPlane::Left), getPlane(FrustumPlane::Top)),
    GeometryUtils::getPlanesIntersection(getPlane(FrustumPlane::Far), getPlane(FrustumPlane::Left), getPlane(FrustumPlane::Bottom)),
    GeometryUtils::getPlanesIntersection(getPlane(FrustumPlane::Far), getPlane(FrustumPlane::Right), getPlane(FrustumPlane::Bottom)),
    GeometryUtils::getPlanesIntersection(getPlane(FrustumPlane::Far), getPlane(FrustumPlane::Right), getPlane(FrustumPlane::Top)),
    GeometryUtils::getPlanesIntersection(getPlane(FrustumPlane::Far), getPlane(FrustumPlane::Left), getPlane(FrustumPlane::Top)),
  };
}

Frustum Frustum::extractFromViewProjection(const glm::mat4x4& view, const glm::mat4x4 projection)
{
  glm::mat4x4 viewProjection = projection * view;

  Frustum frustum;

  frustum.setPlane(FrustumPlane::Left, Plane(glm::vec3(
    viewProjection[0][3] + viewProjection[0][0],
    viewProjection[1][3] + viewProjection[1][0],
    viewProjection[2][3] + viewProjection[2][0]),
    viewProjection[3][3] + viewProjection[3][0]));

  frustum.setPlane(FrustumPlane::Right, Plane(glm::vec3(
    viewProjection[0][3] - viewProjection[0][0],
    viewProjection[1][3] - viewProjection[1][0],
    viewProjection[2][3] - viewProjection[2][0]),
    viewProjection[3][3] - viewProjection[3][0]));

  frustum.setPlane(FrustumPlane::Top, Plane(glm::vec3(
    viewProjection[0][3] - viewProjection[0][1],
    viewProjection[1][3] - viewProjection[1][1],
    viewProjection[2][3] - viewProjection[2][1]),
    viewProjection[3][3] - viewProjection[3][1]));

  frustum.setPlane(FrustumPlane::Bottom, Plane(glm::vec3(
    viewProjection[0][3] + viewProjection[0][1],
    viewProjection[1][3] + viewProjection[1][1],
    viewProjection[2][3] + viewProjection[2][1]),
    viewProjection[3][3] + viewProjection[3][1]));

  frustum.setPlane(FrustumPlane::Near, Plane(glm::vec3(
    viewProjection[0][3] + viewProjection[0][2],
    viewProjection[1][3] + viewProjection[1][2],
    viewProjection[2][3] + viewProjection[2][2]),
    viewProjection[3][3] + viewProjection[3][2]));

  frustum.setPlane(FrustumPlane::Far, Plane(glm::vec3(
    viewProjection[0][3] - viewProjection[0][2],
    viewProjection[1][3] - viewProjection[1][2],
    viewProjection[2][3] - viewProjection[2][2]),
    viewProjection[3][3] - viewProjection[3][2]));

  for (size_t planeIndex = 0; planeIndex < 6; planeIndex++) {
    frustum.getPlane(planeIndex).normalize();
  }

  return frustum;
}

Frustum Frustum::extractFromCorners(const std::array<glm::vec3, 8>& corners) {

    return Frustum({GeometryUtils::getPlaneBy3Points(corners[7], corners[3], corners[0]), //left
                   GeometryUtils::getPlaneBy3Points(corners[6], corners[5], corners[1]), //right
                   GeometryUtils::getPlaneBy3Points(corners[3], corners[7], corners[6]), //top
                   GeometryUtils::getPlaneBy3Points(corners[1], corners[5], corners[4]), //bottom
                   GeometryUtils::getPlaneBy3Points(corners[3], corners[2], corners[1]), //near
                   GeometryUtils::getPlaneBy3Points(corners[6], corners[7], corners[4])}); //far
}

Sphere::Sphere()
{

}

Sphere::Sphere(const glm::vec3& origin, float radius)
  : m_origin(origin), m_radius(radius)
{

}

void Sphere::setOrigin(const glm::vec3& origin)
{
  m_origin = origin;
}

const glm::vec3& Sphere::getOrigin() const
{
  return m_origin;
}

void Sphere::setRadius(float radius)
{
  m_radius = radius;
}

float Sphere::getRadius() const
{
  return m_radius;
}

void Sphere::applyTransform(const glm::mat4& transformationMatrix)
{
  glm::vec3 origin = glm::vec3(transformationMatrix * glm::vec4(m_origin, 1.0));

  glm::vec3 scale = MathUtils::extractScale2(transformationMatrix);
  float radiusFactor = glm::sqrt(glm::max(glm::max(scale.x, scale.y), scale.z));
  float radius = m_radius * radiusFactor;

  m_origin = origin;
  m_radius = radius;
}

float GeometryUtils::calculateDistance(const glm::vec3& v1, const glm::vec3& v2)
{
  return glm::distance(v1, v2);
}

float GeometryUtils::calculateDistance(const glm::vec3& point, const Plane& plane)
{
  return abs(GeometryUtils::calculateSignedDistance(point, plane));
}

float GeometryUtils::calculateSignedDistance(const glm::vec3& point, const Plane& plane)
{
  return glm::dot(plane.getNormal(), point) + plane.getDistance();
}

bool GeometryUtils::isSphereFrustumIntersecting(const Sphere& sphere, const Frustum& frustum)
{
  for (size_t sideIndex = 0; sideIndex < 6; sideIndex++) {
    const Plane& plane = frustum.getPlane(sideIndex);

    if (calculateSignedDistance(sphere.getOrigin(), plane) < -sphere.getRadius()) {
      return false;
    }
  }

  return true;
}

glm::vec3 GeometryUtils::getPlanesIntersection(const Plane& p1, const Plane& p2, const Plane& p3)
{
  glm::mat3 normalMatrix(p1.getNormal(), p2.getNormal(), p3.getNormal());
  glm::mat3 inversedNormalMatrix(glm::inverse(normalMatrix));

  return glm::vec3(-p1.getDistance(), -p2.getDistance(), -p3.getDistance()) * inversedNormalMatrix;
}

AABB::AABB()
  : m_min(glm::vec3(0.0f, 0.0f, 0.0f)),
    m_max(glm::vec3(0.0f, 0.0f, 0.0f))
{

}

AABB::AABB(const glm::vec3& min, const glm::vec3& max)
  : m_min(min), m_max(max)
{

}

void AABB::setMin(const glm::vec3& min)
{
  m_min = min;
}

const glm::vec3& AABB::getMin() const
{
  return m_min;
}

void AABB::setMax(const glm::vec3& max)
{
  m_max = max;
}

const glm::vec3& AABB::getMax() const
{
  return m_max;
}

glm::vec3 AABB::getSize() const
{
  return m_max - m_min;
}

Sphere AABB::toSphere() const
{
  glm::vec3 size = getSize();
  float radius = glm::length(size) * 0.5f;
  glm::vec3 origin = (m_max + m_min) * 0.5f;

  return Sphere(origin, radius);
}

std::array<glm::vec3, 8> AABB::getCorners() const
{
  return {
    glm::vec3{m_min.x, m_min.y, m_min.z},
    glm::vec3{m_max.x, m_min.y, m_min.z},
    glm::vec3{m_min.x, m_max.y, m_min.z},
    glm::vec3{m_min.x, m_min.y, m_max.z},

    glm::vec3{m_max.x, m_max.y, m_max.z},
    glm::vec3{m_min.x, m_max.y, m_max.z},
    glm::vec3{m_max.x, m_min.y, m_max.z},
    glm::vec3{m_max.x, m_max.y, m_min.z},
  };
}

void AABB::applyTransform(const glm::mat4& transformationMatrix)
{
  glm::vec3 newMin(std::numeric_limits<float>::max());
  glm::vec3 newMax(std::numeric_limits<float>::lowest());

  for (glm::vec3 corner : getCorners()) {
    glm::vec4 newCorner = transformationMatrix * glm::vec4(corner, 1.0f);

    newMin.x = std::fminf(newMin.x, newCorner.x);
    newMin.y = std::fminf(newMin.y, newCorner.y);
    newMin.z = std::fminf(newMin.z, newCorner.z);

    newMax.x = std::fmaxf(newMax.x, newCorner.x);
    newMax.y = std::fmaxf(newMax.y, newCorner.y);
    newMax.z = std::fmaxf(newMax.z, newCorner.z);
  }

  m_min = newMin;
  m_max = newMax;
}

glm::vec3 AABB::getOrigin() const
{
  return (m_max + m_min) * 0.5f;
}

bool GeometryUtils::isAABBFrustumIntersecting(const AABB& aabb, const Frustum& frustum)
{
  const auto& corners = aabb.getCorners();

  for (size_t sideIndex = 0; sideIndex < 6; sideIndex++) {
    const Plane& plane = frustum.getPlane(sideIndex);

    size_t pointsOutsideFrustum = 0;

    for (const glm::vec3& corner : corners) {
      pointsOutsideFrustum += calculateSignedDistance(corner, plane) < 0.0f;
    }

    if (pointsOutsideFrustum == 8) {
      return false;
    }
  }

  return true;
}

Plane GeometryUtils::getPlaneBy3Points(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    glm::vec3 AB = B - A;
    glm::vec3 AC = C - A;

    glm::vec3 normal = glm::cross(AB, AC);

    float d = - (normal.x * A.x + normal.y * A.y + normal.z * A.z);

    return Plane::fromUnnormalized(normal, d);
}

AABB GeometryUtils::restoreAABBByVerticesList(const std::vector<glm::vec3>& vertices) {
    SW_ASSERT(vertices.size() > 1);

    glm::vec3 min = vertices[0];
    glm::vec3 max = vertices[0];

    for (const auto& vertex : vertices) {
        min.x = glm::min(min.x, vertex.x);
        max.x = glm::max(max.x, vertex.x);

        min.y = glm::min(min.y, vertex.y);
        max.y = glm::max(max.y, vertex.y);

        min.z = glm::min(min.z, vertex.z);
        max.z = glm::max(max.z, vertex.z);
    }

    return AABB(min, max);
}

Sphere GeometryUtils::restoreSphereByVerticesList(const std::vector<glm::vec3>& vertices) {
    SW_ASSERT(vertices.size() > 1);

    size_t index = 1;
    float distance = glm::distance(vertices[0], vertices[index]);

    for (size_t vertex_index = 2; vertex_index< vertices.size(); vertex_index++) {
        float current_distance = glm::distance(vertices[0], vertices[vertex_index]);

        if(current_distance > distance) {
            distance = current_distance;
            index = vertex_index;
        }
    }

    glm::vec3 origin = (vertices[0] + vertices[index]) / 2.0f;
    float radius = distance / 2.0f;

    return Sphere(origin, radius);
}

AABB GeometryUtils::mergeAABB(const AABB& aabb1, const AABB& aabb2)
{
  glm::vec3 min = glm::vec3(
    glm::min(aabb1.getMin().x, aabb2.getMin().x),
    glm::min(aabb1.getMin().y, aabb2.getMin().y),
    glm::min(aabb1.getMin().z, aabb2.getMin().z)
  );
  glm::vec3 max = glm::vec3(
    glm::max(aabb1.getMax().x, aabb2.getMax().x),
    glm::max(aabb1.getMax().y, aabb2.getMax().y),
    glm::max(aabb1.getMax().z, aabb2.getMax().z)
  );

  return AABB(min, max);
}
