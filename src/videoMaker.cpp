#define NB_BASE_DE_DONNEE 9765
#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <float.h>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;


void generer_mosaique(const string& inputPath, const string& outputPath, int tailleBloc, int repetition,
                      const vector<ImagetteCouleur>& baseImagettes) {
    int nH, nW, nTaille, nW3;
    OCTET *ImgIn, *ImgOut;

    lire_nb_lignes_colonnes_image_ppm(const_cast<char*>(inputPath.c_str()), &nH, &nW);
    nTaille = nH * nW;
    nW3 = 3 * nW;
    int nTaille3 = nTaille * 3;

    allocation_tableau(ImgIn, OCTET, nTaille3);
    lire_image_ppm(const_cast<char*>(inputPath.c_str()), ImgIn, nH * nW);
    allocation_tableau(ImgOut, OCTET, nTaille3);

    vector<ImagetteCouleur> listeImagettes = baseImagettes;

    for (int i = 0; i <= nH - tailleBloc; i += tailleBloc) {
        for (int j = 0; j <= nW3 - 3 * tailleBloc; j += 3 * tailleBloc) {
            int pixelDepart = i * nW3 + j;
            vector<float> moyen_bloc(3, 0.f);

            for (int k = 0; k < tailleBloc; k++) {
                for (int p = 0; p < tailleBloc * 3; p += 3) {
                    for (int canal = 0; canal < 3; canal++) {
                        moyen_bloc[canal] += ImgIn[pixelDepart + k * nW3 + p + canal];
                    }
                }
            }
            for (int canal = 0; canal < 3; canal++) {
                moyen_bloc[canal] /= float(tailleBloc * tailleBloc);
            }

            float distanceMin = FLT_MAX;
            int best_imagette_id = -1;
            if (repetition) {
                for (const ImagetteCouleur& imagette : listeImagettes) {
                    float distance = 0.f;
                    for (int canal = 0; canal < 3; canal++) {
                        distance += pow(imagette.moyen[canal] - moyen_bloc[canal], 2);
                    }
                    distance = sqrt(distance);
                    if (distance < distanceMin) {
                        distanceMin = distance;
                        best_imagette_id = imagette.ID;
                    }
                }
            } else {
                for (const ImagetteCouleur& imagette : listeImagettes) {
                    if (imagette.isUsed) continue;
                    float distance = 0.f;
                    for (int canal = 0; canal < 3; canal++) {
                        distance += pow(imagette.moyen[canal] - moyen_bloc[canal], 2);
                    }
                    distance = sqrt(distance);
                    if (distance < distanceMin) {
                        distanceMin = distance;
                        best_imagette_id = imagette.ID;
                    }
                }
                for (ImagetteCouleur& imagette : listeImagettes) {
                    if (imagette.ID == best_imagette_id) {
                        imagette.isUsed = true;
                        break;
                    }
                }
            }

            OCTET *ImgIn_imagette, *ImgOut_imagette;
            int nH_imagette, nW_imagette, nTaille_imagette;
            loadImagette_cou(best_imagette_id, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
            allocation_tableau(ImgOut_imagette, OCTET, tailleBloc * tailleBloc * 3);
            resize_imagetteCouleur(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);

            for (int k = 0; k < tailleBloc; k++) {
                for (int p = 0; p < tailleBloc * 3; p += 3) {
                    for (int canal = 0; canal < 3; canal++) {
                        ImgOut[pixelDepart + k * nW3 + p + canal] = ImgOut_imagette[k * tailleBloc * 3 + p + canal]*0.7 + 0.3*ImgIn[pixelDepart + k * nW3 + p + canal];
                    }
                }
            }
            free(ImgIn_imagette);
            free(ImgOut_imagette);
        }
    }

    ecrire_image_ppm(const_cast<char*>(outputPath.c_str()), ImgOut, nH, nW);
    free(ImgIn);
    free(ImgOut);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: dossierImageIn dossierImageOut taille_bloc\n");
        return 1;
    }

    string dossierIn = argv[1];
    string dossierOut = argv[2];
    int tailleBloc;
    sscanf(argv[3], "%d", &tailleBloc);

    vector<ImagetteCouleur> baseImagettes;
    for (int idImagette = 1; idImagette <= NB_BASE_DE_DONNEE; idImagette++) {
        OCTET *ImgIn_imagette, *ImgOut_imagette;
        int nH, nW, nTaille;
        vector<float> moyen(3, 0.f);
        loadImagette_cou(idImagette, ImgIn_imagette, nH, nW, nTaille);
        allocation_tableau(ImgOut_imagette, OCTET, tailleBloc * tailleBloc * 3);
        resize_imagetteCouleur(ImgIn_imagette, nH, nW, ImgOut_imagette, tailleBloc, tailleBloc);

        for (int j = 0; j < tailleBloc * tailleBloc * 3; j += 3) {
            for (int canal = 0; canal < 3; canal++) {
                moyen[canal] += ImgOut_imagette[j + canal];
            }
        }
        for (int canal = 0; canal < 3; canal++) {
            moyen[canal] /= (float)(tailleBloc * tailleBloc);
        }

        baseImagettes.push_back({idImagette, moyen, {}, {}, false});
        free(ImgIn_imagette);
        free(ImgOut_imagette);
    }

    for (const auto& entry : fs::directory_iterator(dossierIn)) {
        if (entry.path().extension() == ".ppm") {
            string inputPath = entry.path().string();
            string outputPath = (fs::path(dossierOut) / (entry.path().stem().string() + "_mosaique.ppm")).string();
            cout << "\nTraitement de : " << inputPath << endl;
            generer_mosaique(inputPath, outputPath, tailleBloc, 0, baseImagettes);
        }
    }

    return 0;
}
