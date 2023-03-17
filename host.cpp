#include <string>
#include <vector>
#include <format>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "types.h"

int run_host(const std::string& tcp_address) {

    // open socket
    nng_socket socket;
    nng_listener listener;
    int nng_result = 0;
    if ((nng_result = nng_pub0_open(&socket)) != 0) {
        return -1;
    }
    if ((nng_result = nng_listen(socket, tcp_address.c_str(), &listener, 0)) != 0) {
        nng_close(socket);
        return -2;
    }

    // WINDOW & GL SETUP
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "RealSense Test", nullptr, nullptr);
    glfwSetWindowPos(window, 100, 100);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glewInit();
    glClearColor(0.1f, 0.1f, 0.1f, 1);

    // IMGUI SETUP
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    float capture_timer = 0;
    bool recording = false;

    RecordPacket record_packet;

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        unsigned long long client_count = 0;
        nng_stat* stats = nullptr;
        nng_stats_get(&stats);
        nng_stat* stat = nng_stat_find(stats, "socket");
        if (stat != nullptr) {
            stat = nng_stat_find(stat, "pipes");
            if (stat != nullptr)
                client_count = nng_stat_value(stat);
        }
        nng_stats_free(stats);

        if (recording) {
            capture_timer += ImGui::GetIO().DeltaTime;
        }

        bool wasRecording = recording;

        int minutes = (int)(capture_timer / 60);
        int seconds = (int)(capture_timer - minutes * 60);
        int milliseconds = (int)((capture_timer - (int)capture_timer) * 100);

        auto minutes_str = minutes < 10 ? std::format("0{}", minutes) : std::format("{}", minutes);
        auto seconds_str = seconds < 10 ? std::format("0{}", seconds) : std::format("{}", seconds);
        auto milliseconds_str = milliseconds < 10 ? std::format("0{}", milliseconds) : std::format("{}", milliseconds);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Recording");

        ImGui::Text("Host Address:");
        ImGui::SameLine();
        ImGui::Text(tcp_address.c_str());
        ImGui::Text("Connected Clients:");
        ImGui::SameLine();
        ImGui::Text("%d", client_count);
        if (ImGui::Button("Close Clients")) {

            recording = false;
            record_packet.record = 0;
            record_packet.close = 1;
            nng_send(socket, &record_packet, sizeof(RecordPacket), 0);
        }
        ImGui::Separator();

        if (wasRecording)
            ImGui::BeginDisabled();

        ImGui::Text("Record Delay:");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::InputFloat("##delay", &record_packet.delay);
        ImGui::PopItemWidth();

        ImGui::Text("Record Duration:");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::InputFloat("##duration", &record_packet.duration);
        ImGui::PopItemWidth();

        ImGui::Text("Desired FPS:");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::InputInt("##fps", &record_packet.desiredFPS);
        ImGui::PopItemWidth();

        ImGui::Text("Name Prefix:");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##prefix", record_packet.prefixPath, 16);
        ImGui::PopItemWidth();

        ImGui::Separator();

     //   if (wasRecording)
     //       ImGui::BeginDisabled();

        if (ImGui::Button("Start Recording")) {
            recording = true;
            capture_timer = 0;

            record_packet.record = 1;
            nng_send(socket, &record_packet, sizeof(RecordPacket), 0);
        }

        ImGui::SameLine();

        if (!wasRecording)
            ImGui::BeginDisabled();
        else
            ImGui::EndDisabled();

        if (ImGui::Button("Stop Recording")) {
            recording = false;

            record_packet.record = 0;
            nng_send(socket, &record_packet, sizeof(RecordPacket), 0);
        }

        if (!wasRecording)
            ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Text("Duration:");
        ImGui::SameLine();
        ImGui::Text("%s:%s:%s", minutes_str.c_str(), seconds_str.c_str(), milliseconds_str.c_str());

        ImGui::End();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    nng_close(socket);

	return 0;
}