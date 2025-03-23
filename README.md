# Projet Mosaïque

## Viguier Killian - Wang Xihao

Pour utiliser cuda: 
-   
    1. nvcc -o exe/nom_program_cuda nom_fichier_cuda.cu
    2. ./exe/nom_program_cuda img_in_out/... ... ...

Si tu obtiens une erreur, essaies: 
-   
    1. nvidia-smi
    2. trouver ton type de GPU dans ce tableau (ex. 0   NVIDIA A100-SXM4-40GB   off ...)
    3. [Clicker ce lien et trouver le flag correspondant](https://arnon.dk/matching-sm-architectures-arch-and-gencode-for-various-nvidia-cards/)
    4. Re-compiler avec : nvcc -arch=sm_XX -o exe/nom_program_cuda nom_fichier_cuda.cu
    5. executer