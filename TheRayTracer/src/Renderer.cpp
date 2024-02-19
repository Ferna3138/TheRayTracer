#include "Renderer.h"
#include "Walnut/Random.h"
#include <memory>

namespace utils {
	
	static uint32_t ConvertToRGBA(const glm::vec4& colour) {
		uint8_t r = uint8_t(colour.r * 255.0f);
		uint8_t g = uint8_t(colour.g * 255.0f);
		uint8_t b = uint8_t(colour.b * 255.0f);
		uint8_t a = uint8_t(colour.a * 255.0f);

		//0xff000000 Remember alpha channel in hex code is situated at the beggining
		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;

		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height) {
	if (m_FinalImage) {
		//No resize needed
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		
		m_FinalImage->Resize(width, height);
	}
	else {
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

}

void Renderer::Render(const Scene& scene, const Camera& camera) {
	Ray ray;
	ray.Origin = camera.GetPosition();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {

			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage -> GetWidth()];

			glm::vec4 colour = TraceRay(scene ,ray);
			colour = glm::clamp(colour, glm::vec4(0.0f), glm::vec4(1.0f)); //Make sure colour stays within 0 and 1
			m_ImageData[x + y * m_FinalImage->GetWidth()] = utils::ConvertToRGBA(colour);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray) {
	//Check whether or not there are any spheres in the scene
	if (scene.Spheres.size() == 0)
		return glm::vec4(0, 0, 0, 1);


	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();

	for (const Sphere& sphere : scene.Spheres) {
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2 * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		//Quadratic formula discriminant b^2 - 4ac
		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0.0f)
			continue;

		// (-b +- sqrt(discriminant)) / 2a
			//float t0 = (-b + glm::sqrt(discriminant)) / 2.0f * a; //Second hit at distance (currently unused)
		float closestT = (-b - glm::sqrt(discriminant)) / 2.0f * a;

		if (closestT < hitDistance) {
			hitDistance = closestT;
			closestSphere = &sphere;
		}
	}

	if (closestSphere == nullptr)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	glm::vec3 origin = ray.Origin - closestSphere -> Position;
	glm::vec3 hitPoint = origin + ray.Direction * hitDistance;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

	float lightIntensity = glm::max(glm::dot(normal, -lightDir), 0.0f); //cos(angle between normal and -lightDir)

	glm::vec3 sphereColour = closestSphere -> Albedo;
	sphereColour *= lightIntensity;
	return glm::vec4(sphereColour, 1.0f);
}
