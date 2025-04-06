#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <array>
#include <cmath>
#include <float.h>
#include <chrono>
#define NB_BASE_DE_DONNEE 10000
using namespace std;



int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, tailleBloc;
  std::vector<Imagette> listeImagettes;

  if (argc != 4) 
    {
      printf("Usage: ImageIn.pgm ImageOut.pgm taille_bloc \n"); 
      exit (1) ;
    }

  std::chrono::time_point<std::chrono::high_resolution_clock> _t0 = std::chrono::high_resolution_clock::now();

  sscanf (argv[1],"%s",cNomImgLue) ;
  sscanf (argv[2],"%s",cNomImgEcrite);
  sscanf (argv[3],"%d",&tailleBloc);

  OCTET *ImgIn, *ImgOut;

  lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
  nTaille = nH * nW;

  allocation_tableau(ImgIn, OCTET, nTaille);
  lire_image_pgm(cNomImgLue, ImgIn, nTaille);
  allocation_tableau(ImgOut, OCTET, nTaille);

  // Charger l'histogramme de chaque imagette
  for(int idImagette = 1; idImagette <= NB_BASE_DE_DONNEE; idImagette++){
    OCTET *ImgIn_imagette, *ImgOut_imagette;
    std::array<int,256> histoImagette{0};
    int nH_imagette, nW_imagette, nTaille_imagette;
    
    loadImagette(idImagette, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
    allocation_tableau(ImgOut_imagette, OCTET, tailleBloc * tailleBloc);
    resize_imagette(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
    
    for(int j = 0; j < tailleBloc * tailleBloc; j++){
      histoImagette[ImgOut_imagette[j]]++;
    }

    listeImagettes.push_back({idImagette, 0.f, histoImagette, 0});
    free(ImgIn_imagette);free(ImgOut_imagette);
  }

  // Remplacement des blocs de l'image originale
  for(int i = 0; i <= nH - tailleBloc; i += tailleBloc){
    for(int j = 0; j <= nW - tailleBloc; j += tailleBloc){
      int pixelDepart = i * nW + j;
      std::array<int, 256> histoBloc{0};

      for(int k = 0; k < tailleBloc; k++){
        for(int p = 0; p < tailleBloc; p++){
          histoBloc[ImgIn[pixelDepart + k * nW + p]]++;
        }
      }

      float distanceMin = FLT_MAX;
      int best_imagette_id = -1;

      for(Imagette &imagette : listeImagettes){
        float distanceBhattacharyya = 0.f;
        for(int couleur = 0; couleur < 256; couleur++){
          distanceBhattacharyya += sqrt(histoBloc[couleur] * imagette.histo[couleur]);
        }
        distanceBhattacharyya = -log(distanceBhattacharyya);

        if(distanceBhattacharyya < distanceMin){
          distanceMin = distanceBhattacharyya;
          best_imagette_id = imagette.ID;
        }
      }

      // Charger l'imagette sélectionnée et la réduire si nécessaire
      OCTET *ImgOut_imagette;
      OCTET *ImgIn_imagette;
      int nH_imagette, nW_imagette, nTaille_imagette;
      
      loadImagette(best_imagette_id, ImgIn_imagette, nH_imagette, nW_imagette, nTaille_imagette);
      allocation_tableau(ImgOut_imagette, OCTET, tailleBloc * tailleBloc);
      resize_imagette(ImgIn_imagette, nH_imagette, nW_imagette, ImgOut_imagette, tailleBloc, tailleBloc);
      
      // Remplacer les pixels du bloc par ceux de l'imagette
      for(int k = 0; k < tailleBloc; k++){
        for(int p = 0; p < tailleBloc; p++){
          ImgOut[pixelDepart + k * nW + p] = ImgOut_imagette[k * tailleBloc + p];
        }
      }
      
      free(ImgIn_imagette);
      free(ImgOut_imagette);
    }
  }

  std::chrono::time_point<std::chrono::high_resolution_clock> _t1 = std::chrono::high_resolution_clock::now();
  std::cout << "Photo-mosaïque produite en : "<<std::chrono::duration<double>(_t1-_t0).count() << "s" << std::endl;

  std::cout<<"PSNR par rapport à l'image d'entrée : "<<calculer_PSNR(ImgIn, ImgOut, nTaille)<<"dB"<<std::endl;

  ecrire_image_pgm(cNomImgEcrite, ImgOut, nH, nW);
  free(ImgIn);
  free(ImgOut);

  return 1;
}
