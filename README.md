# Projet Mosaïque

## Viguier Killian - Wang Xihao

Pour utiliser cuda: 
-   
    nvcc -o exe/nom_program_cuda nom_fichier_cuda.cu

    ./exe/nom_program_cuda img_in_out/... ... ...

Si tu obtiens une erreur, essaies: 
-   
    nvidia-smi

- trouver ton type de GPU dans ce tableau (ex. 0   NVIDIA A100-SXM4-40GB   off ...)

    [Clicker ce lien et trouver le flag(-arch=sm_XX) correspondant](https://arnon.dk/matching-sm-architectures-arch-and-gencode-for-various-nvidia-cards)

Et puis:
-   
    nvcc -arch=sm_XX -o exe/nom_program_cuda nom_fichier_cuda.cu

    ./exe/nom_program_cuda img_in_out/... ... ...