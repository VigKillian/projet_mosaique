// Example: GUI with Dear ImGui + file selector + image preview + run C++ program (non-blocking)
// Dependencies: ImGui, GLFW, stb_image.h (for image loading)

#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_glfw.h"
#include "./imgui/imgui_impl_opengl3.h"
#include "./imgui/ImGuiFileDialog.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <cstdlib>  // for system()
#include <thread>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fs = std::filesystem;

GLuint LoadTextureFromFile(const char* filename, int& width, int& height) {
    unsigned char* image_data = stbi_load(filename, &width, &height, NULL, 3);
    if (!image_data) return 0;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);
    return tex;
}

float ReadProgressFromFile(const std::string& filename) {
    std::ifstream f(filename);
    float progress = 0.0f;
    if (f >> progress) return progress;
    return 0.0f;
}

int main() {
    if (!glfwInit()) return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Photo Mosaïque UI", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    char inputPath[256] = "./img_in_out/girafes.pgm";
    char outputPath[256] = "./img_in_out/app_sort/out.pgm";
    char tailleBlocStr[10] = "32";
    int imageWidth = 0, imageHeight = 0;
    GLuint imageTex = 0;

    std::thread workerThread;
    bool taskRunning = false;
    bool needReload = false;
    bool repetition_gris = false;
    bool repetition_couleur = false;


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Photo Mosaïque Interface");
        if (needReload) {
            if (imageTex) glDeleteTextures(1, &imageTex);
            imageTex = LoadTextureFromFile(outputPath, imageWidth, imageHeight);
            needReload = false;
        }
//==========================================================================================================
        if (ImGui::Button("Choisir un fichier")) {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choisir une image", ".pgm,.ppm", config);
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::cout << "Fichier sélectionné : " << filePath << std::endl;
                strncpy(inputPath, filePath.c_str(), sizeof(inputPath));
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::InputText("Taille Bloc", tailleBlocStr, IM_ARRAYSIZE(tailleBlocStr));

        if (ImGui::Button("Charger l'image")) {
            if (imageTex) glDeleteTextures(1, &imageTex);
            imageTex = LoadTextureFromFile(inputPath, imageWidth, imageHeight);
        }
//==========================================================================================================
        if (imageTex) {
            ImGui::Text("Aperçu de l'image:");
            ImGui::Image((ImTextureID)(intptr_t)imageTex, ImVec2(512, 512 * imageHeight / imageWidth));
        }

        auto runCommandAsync = [&](const std::string& cmd) {
            if (workerThread.joinable()) workerThread.join();
            taskRunning = true;
            workerThread = std::thread([&, cmd]() {
                system(cmd.c_str());
                needReload = true;
                taskRunning = false;
            });
        };
//==========================================================================================================
        ImGui::Separator();
        ImGui::Text("Les méthodes de niveau de gris");
        ImGui::Checkbox("mode repetition gris", &repetition_gris);
        if (ImGui::Button("Par moyen gris") && !taskRunning) {
            std::string exec = repetition_gris ? "./exe/gris_moyen" : "./exe/gris_moyen_sanRep";
            std::string cmd = exec + std::string(" ") + inputPath + " " + outputPath + " " + tailleBlocStr;
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Par histo gris") && !taskRunning) {
            std::string cmd = "./exe/gris_histo " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr);
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Par distribution gris") && !taskRunning) {
            std::string cmd;
            if(repetition_gris)
                cmd = "./exe/gris_distribution " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr) + " 1";
            else
                cmd = "./exe/gris_distribution " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr) + " 0";
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Triche spécification") && !taskRunning) {
            std::string exec = "./exe/gris_moyen_specif" ;
            std::string cmd = exec + std::string(" ") + inputPath + " " + outputPath + " " + tailleBlocStr;
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        }
//==========================================================================================================
        ImGui::Separator();
        ImGui::Text("Les méthodes de niveau de couleur");
        ImGui::Checkbox("mode repetition couleur", &repetition_couleur);
        if (ImGui::Button("Par moyen couleur") && !taskRunning) {
            std::string cmd;
            if(repetition_couleur)
                cmd = "./exe/couleur_moyen " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr) + " 1";
            else
                cmd = "./exe/couleur_moyen " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr) + " 0";
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Par histo couleur") && !taskRunning) {
            std::string cmd;
            if(repetition_couleur)
                cmd = "./exe/couleur_histo " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr) + " 1";
            else
                cmd = "./exe/couleur_histo " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr) + " 0";
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Par distribution couleur") && !taskRunning) {
            std::string cmd;
            if(repetition_couleur)
                cmd = "./exe/couleur_distribution " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr) + " 1";
            else
                cmd = "./exe/couleur_distribution " + std::string(inputPath) + " " + std::string(outputPath) + " " + std::string(tailleBlocStr) + " 0";
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Triche moyenne pondérée") && !taskRunning) {
            std::string exec = "./exe/couleur_moyen_triche" ;
            std::string cmd;
            if(repetition_couleur)
                cmd = exec + std::string(" ") + inputPath + " " + outputPath + " " + tailleBlocStr + " 1";
            else
                cmd = exec + std::string(" ") + inputPath + " " + outputPath + " " + tailleBlocStr + " 0";
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        }
//==========================================================================================================
        ImGui::Separator();
        ImGui::Text("Autres approches: détecter l'objet");
        if (ImGui::Button("moyen avancé") && !taskRunning) {
            std::string exec = "./exe/couleur_moyen_avance" ;
            std::string cmd;
            cmd = exec + std::string(" ") + inputPath + " " + outputPath + " " + tailleBlocStr + " 0";
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        } 
        ImGui::SameLine();
        if (ImGui::Button("distribution avancé") && !taskRunning) {
            std::string exec = "./exe/couleur_distrib_avancee" ;
            std::string cmd;
            cmd = exec + std::string(" ") + inputPath + " " + outputPath + " "+"./img_in_out/cou_dis/poisson_seuille.pgm " + tailleBlocStr + " 0";
            std::cout << "Commande: " << cmd << std::endl;
            runCommandAsync(cmd);
        } 
//==========================================================================================================
        if (taskRunning) {
            float progress0 = ReadProgressFromFile("progress0.txt");
            ImGui::Text("\nLeture d'imagette et changement de taille en cours...");
            ImGui::ProgressBar(progress0, ImVec2(0.0f, 0.0f));
            
            float progress1 = ReadProgressFromFile("progress1.txt");
            ImGui::Text("\nTraitement en cours...");
            ImGui::ProgressBar(progress1, ImVec2(0.0f, 0.0f));
        }

        ImGui::End();
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    if (workerThread.joinable()) workerThread.join();
    if (imageTex) glDeleteTextures(1, &imageTex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}