#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

const float PI = 3.14159265358979323846f;
const float TAU = 2.0f * PI;
const int WIDTH = 1200;
const int HEIGHT = 600;

struct Point3D {
    sf::Vector3f position;
    float radius;
    sf::Color color;
    float originalU, originalV; // Lưu tọa độ gốc để tính animation
};

class PumpkinTorusRenderer {
private:
    sf::RenderWindow window;
    float time;
    sf::Clock clock;
    sf::Font font;
    sf::Text infoText;

    std::vector<Point3D> points;
    std::vector<std::vector<sf::Vector3f>> meridians; // Lưu các đường kinh tuyến để vẽ đường

    // Camera parameters
    float cameraDistance;
    float cameraAngleX, cameraAngleY;
    sf::Vector2f lastMousePos;

    // Torus parameters
    float R;  // Bán kính chính
    float r;  // Bán kính ống
    int numMeridians;   // Số kinh tuyến
    int numParallels;   // Số điểm trên mỗi kinh tuyến

    // Pumpkin parameters
    int pumpkinSegments; // Số múi bí ngô
    float pumpkinDepth;  // Độ sâu của múi

public:
    PumpkinTorusRenderer() :
        window(sf::VideoMode(WIDTH, HEIGHT), "Pumpkin Torus - SFML"),
        time(0.0f),
        cameraDistance(350.0f),
        cameraAngleX(0.0f),
        cameraAngleY(0.0f),
        R(80.0f),
        r(40.0f),
        numMeridians(18),     // 18 kinh tuyến tạo thành các múi
        numParallels(60),     // 60 điểm trên mỗi kinh tuyến
        pumpkinSegments(12),  // 12 múi bí ngô
        pumpkinDepth(0.25f)   // Độ sâu của múi
    {
        window.setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf")) {
            font = sf::Font();
        }

        infoText.setFont(font);
        infoText.setCharacterSize(14);
        infoText.setFillColor(sf::Color::White);
        infoText.setPosition(10, 10);

        generatePumpkinTorus();
    }

    void generatePumpkinTorus() {
        points.clear();
        meridians.clear();

        // Tạo hình bí ngô bằng cách biến dạng torus
        for (int i = 0; i < numMeridians; i++) {
            std::vector<sf::Vector3f> meridianPoints;
            float u = static_cast<float>(i) / numMeridians * TAU;

            // Hiệu ứng bí ngô: tạo các múi lồi lõm
            float pumpkinModulation = 1.0f - pumpkinDepth * cos(pumpkinSegments * u);

            for (int j = 0; j < numParallels; j++) {
                float v = static_cast<float>(j) / numParallels * TAU;

                Point3D point;
                point.originalU = u;
                point.originalV = v;

                // HIỆU ỨNG CUỘN QUANH TORUS:
                // Thay vì vòng tròn đơn giản, tạo đường xoắn quanh torus
                float twist = 3.0f; // Số vòng xoắn
                float twistedV = v + twist * u + time * 0.5f;

                // Tọa độ torus cơ bản với hiệu ứng bí ngô
                float currentR = R * pumpkinModulation;

                // Tọa độ torus với xoắn
                point.position.x = (currentR + r * cos(twistedV)) * cos(u);
                point.position.y = (currentR + r * cos(twistedV)) * sin(u);
                point.position.z = r * sin(twistedV);

                // HIỆU ỨNG CUỘN VÀO TRONG LÕI:
                // Tại một số vị trí đặc biệt, đường cong đi vào bên trong
                // Sử dụng hàm để điều khiển độ sâu
                float innerDepth = 0.0f;

                // Tạo các đoạn cuộn vào bên trong tại các múi bí ngô
                int segment = static_cast<int>(u * pumpkinSegments / TAU) % pumpkinSegments;
                float segmentPhase = fmod(u * pumpkinSegments / TAU, 1.0f);

                if (segmentPhase > 0.4f && segmentPhase < 0.6f) {
                    // Ở giữa mỗi múi, đường cong đi sâu vào trong
                    innerDepth = 0.7f * sin((segmentPhase - 0.5f) * PI * 5.0f);
                }

                // Điều chỉnh vị trí để tạo hiệu ứng cuộn vào trong
                point.position.z += innerDepth * r * 2.0f;

                // Kích thước điểm thay đổi theo vị trí
                point.radius = 3.0f + 2.0f * sin(u * 8.0f + v * 4.0f + time * 2.0f);

                // Màu sắc theo múi bí ngô
                float hue = static_cast<float>(segment) / pumpkinSegments;
                point.color = pumpkinColor(hue, u, v);

                points.push_back(point);
                meridianPoints.push_back(point.position);
            }
            meridians.push_back(meridianPoints);
        }

        // Thêm các đường cong đặc biệt tạo hiệu ứng "cuộn vào lõi"
        addCurlingCurves();
    }

    void addCurlingCurves() {
        // Thêm các đường cong đặc biệt cuộn vào trung tâm
        int numSpecialCurves = pumpkinSegments; // Mỗi múi có một đường cuộn

        for (int curve = 0; curve < numSpecialCurves; curve++) {
            float baseU = static_cast<float>(curve) / numSpecialCurves * TAU;

            for (int i = 0; i <= 40; i++) {
                float t = static_cast<float>(i) / 40.0f;

                Point3D point;

                // Tạo đường cong cuộn vào bên trong
                float u = baseU + 0.1f * sin(t * PI * 4.0f + time * 0.3f);
                float progression = t * 2.0f - 1.0f; // từ -1 đến 1

                // Đường cong đi từ ngoài vào trong rồi ra ngoài
                float v = PI * progression + time * 0.5f;

                // Hiệu ứng bí ngô
                float pumpkinModulation = 1.0f - pumpkinDepth * cos(pumpkinSegments * u);
                float currentR = R * pumpkinModulation;

                // Khi đường cong đi vào giữa, nó thu nhỏ lại
                float curveRadius = r * (0.3f + 0.7f * fabs(sin(t * PI)));

                // Tọa độ với hiệu ứng cuộn
                point.position.x = (currentR + curveRadius * cos(v)) * cos(u);
                point.position.y = (currentR + curveRadius * cos(v)) * sin(u);
                point.position.z = curveRadius * sin(v) * 1.5f; // Kéo dài theo trục Z

                // Điểm trên đường cong có kích thước biến đổi
                point.radius = 5.0f + 3.0f * sin(t * 20.0f + time * 3.0f);

                // Màu sáng cho các đường đặc biệt
                point.color = sf::Color(
                    255,  // R
                    static_cast<sf::Uint8>(200 + 55 * sin(time + t * PI * 2.0f)),
                    static_cast<sf::Uint8>(100 + 50 * cos(time * 1.5f))
                );

                points.push_back(point);
            }
        }
    }

    sf::Color pumpkinColor(float hue, float u, float v) {
        // Tạo màu bí ngô: cam, vàng, nâu
        float t = fmod(hue + time * 0.05f, 1.0f);

        float r, g, b;

        if (t < 0.33f) {
            // Cam đậm -> cam nhạt
            float phase = t * 3.0f;
            r = 0.8f + 0.2f * phase;
            g = 0.3f + 0.3f * phase;
            b = 0.1f;
        } else if (t < 0.66f) {
            // Cam -> vàng
            float phase = (t - 0.33f) * 3.0f;
            r = 0.9f + 0.1f * (1.0f - phase);
            g = 0.6f + 0.4f * phase;
            b = 0.1f + 0.1f * phase;
        } else {
            // Vàng -> cam đậm
            float phase = (t - 0.66f) * 3.0f;
            r = 0.8f + 0.2f * phase;
            g = 0.7f - 0.4f * phase;
            b = 0.2f - 0.1f * phase;
        }

        // Thêm hiệu ứng sáng tối theo bề mặt
        float lighting = 0.7f + 0.3f * sin(u * 10.0f + v * 5.0f + time);
        r *= lighting;
        g *= lighting;
        b *= lighting;

        return sf::Color(
            static_cast<sf::Uint8>(r * 255),
            static_cast<sf::Uint8>(g * 255),
            static_cast<sf::Uint8>(b * 255)
        );
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                else if (event.key.code == sf::Keyboard::R) {
                    cameraAngleX = 0.0f;
                    cameraAngleY = 0.0f;
                    cameraDistance = 350.0f;
                }
                else if (event.key.code == sf::Keyboard::Up) {
                    pumpkinSegments++;
                    if (pumpkinSegments > 24) pumpkinSegments = 24;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    pumpkinSegments--;
                    if (pumpkinSegments < 3) pumpkinSegments = 3;
                }
                else if (event.key.code == sf::Keyboard::Left) {
                    pumpkinDepth -= 0.05f;
                    if (pumpkinDepth < 0.0f) pumpkinDepth = 0.0f;
                }
                else if (event.key.code == sf::Keyboard::Right) {
                    pumpkinDepth += 0.05f;
                    if (pumpkinDepth > 0.5f) pumpkinDepth = 0.5f;
                }
                else if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) {
                    numMeridians += 2;
                    if (numMeridians > 36) numMeridians = 36;
                }
                else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) {
                    numMeridians -= 2;
                    if (numMeridians < 6) numMeridians = 6;
                }
            }
            else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0)
                    cameraDistance -= 20.0f;
                else
                    cameraDistance += 20.0f;
                cameraDistance = std::max(150.0f, std::min(800.0f, cameraDistance));
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    lastMousePos = sf::Vector2f(
                        static_cast<float>(event.mouseButton.x),
                        static_cast<float>(event.mouseButton.y)
                    );
                }
            }
            else if (event.type == sf::Event::MouseMoved) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    sf::Vector2f currentMousePos(
                        static_cast<float>(event.mouseMove.x),
                        static_cast<float>(event.mouseMove.y)
                    );

                    sf::Vector2f delta = currentMousePos - lastMousePos;
                    cameraAngleY += delta.x * 0.01f;
                    cameraAngleX += delta.y * 0.01f;

                    lastMousePos = currentMousePos;
                }
            }
        }
    }

    void update() {
        time += 0.015f;
        generatePumpkinTorus();

        std::string info = "PUMPKIN TORUS - Curling Meridians\n";
        info += "Effect: Lines curl into the core and emerge\n";
        info += "Controls:\n";
        info += "- Mouse drag: Rotate camera\n";
        info += "- Mouse wheel: Zoom\n";
        info += "- R: Reset camera\n";
        info += "- UP/DOWN: Pumpkin segments (" + std::to_string(pumpkinSegments) + ")\n";
        info += "- LEFT/RIGHT: Pumpkin depth (" + std::to_string(pumpkinDepth).substr(0, 4) + ")\n";
        info += "- +/-: Number of meridians (" + std::to_string(numMeridians) + ")\n";
        info += "- ESC: Exit";
        infoText.setString(info);
    }

    void render() {
        window.clear(sf::Color(10, 5, 15)); // Nền tối hơn

        // Vẽ các đường nối giữa các điểm trên cùng kinh tuyến
        // Điều này tạo ra hiệu ứng "các đường cong liên tục"
        for (const auto& meridian : meridians) {
            if (meridian.size() < 2) continue;

            for (size_t i = 0; i < meridian.size() - 1; i++) {
                sf::Vector3f pos1 = rotate3D(meridian[i]);
                sf::Vector3f pos2 = rotate3D(meridian[i + 1]);

                sf::Vector2f proj1 = project3D(pos1);
                sf::Vector2f proj2 = project3D(pos2);

                // Tính độ dày đường dựa trên độ sâu
                float zAvg = (pos1.z + pos2.z) / 2.0f;
                float scale = 1.0f - (zAvg + cameraDistance) / (2.0f * cameraDistance);
                float lineThickness = 2.0f * scale;

                if (lineThickness > 0.1f) {
                    // Vẽ đường nối
                    sf::Vertex line[] = {
                        sf::Vertex(proj1, pumpkinColor(0.5f, 0, 0)),
                        sf::Vertex(proj2, pumpkinColor(0.5f, 0, 0))
                    };

                    // Sử dụng primitive lines
                    window.draw(line, 2, sf::Lines);
                }
            }
        }

        // Vẽ các điểm (vẽ sau để nằm trên các đường)
        std::vector<std::pair<float, const Point3D*>> sortedPoints;
        for (const auto& point : points) {
            sf::Vector3f rotated = rotate3D(point.position);
            sortedPoints.push_back(std::make_pair(rotated.z, &point));
        }

        std::sort(sortedPoints.begin(), sortedPoints.end(),
                  [](const std::pair<float, const Point3D*>& a,
                     const std::pair<float, const Point3D*>& b) {
                      return a.first > b.first;
                  });

        for (const auto& pointPair : sortedPoints) {
            const Point3D& point = *pointPair.second;
            sf::Vector3f rotated = rotate3D(point.position);
            sf::Vector2f projected = project3D(rotated);

            float scale = 1.0f - (rotated.z + cameraDistance) / (2.0f * cameraDistance);
            scale = std::max(0.1f, scale);

            float displayRadius = point.radius * scale;

            if (displayRadius > 0.3f &&
                projected.x >= -displayRadius && projected.x <= WIDTH + displayRadius &&
                projected.y >= -displayRadius && projected.y <= HEIGHT + displayRadius) {

                sf::CircleShape circle(displayRadius);
                circle.setPosition(projected.x - displayRadius, projected.y - displayRadius);
                circle.setFillColor(point.color);

                // Thêm hiệu ứng ánh sáng cho các điểm lớn
                if (displayRadius > 2.0f) {
                    circle.setOutlineThickness(1.0f);
                    sf::Color outlineColor = point.color;
                    outlineColor.r = std::min(255, outlineColor.r + 50);
                    outlineColor.g = std::min(255, outlineColor.g + 30);
                    circle.setOutlineColor(outlineColor);
                }

                window.draw(circle);
            }
        }

        window.draw(infoText);
        window.display();
    }

    sf::Vector3f rotate3D(const sf::Vector3f& point) {
        sf::Vector3f result = point;

        // Quay quanh trục Y
        float tempX = result.x * cos(cameraAngleY) - result.z * sin(cameraAngleY);
        float tempZ = result.x * sin(cameraAngleY) + result.z * cos(cameraAngleY);
        result.x = tempX;
        result.z = tempZ;

        // Quay quanh trục X
        float tempY = result.y * cos(cameraAngleX) - result.z * sin(cameraAngleX);
        tempZ = result.y * sin(cameraAngleX) + result.z * cos(cameraAngleX);
        result.y = tempY;
        result.z = tempZ;

        return result;
    }

    sf::Vector2f project3D(const sf::Vector3f& point) {
        float scale = cameraDistance / (cameraDistance + point.z);
        return sf::Vector2f(
            point.x * scale + WIDTH / 2.0f,
            point.y * scale + HEIGHT / 2.0f
        );
    }
};

int main() {
    PumpkinTorusRenderer renderer;
    renderer.run();
    return 0;
}
