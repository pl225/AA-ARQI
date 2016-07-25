#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <papi.h>

#define KB 1024
#define MB 1048576
#define GB 1073741824
#define SETECENTOS 805306368

void gera_elementos (long int n, float *vetor)
{
    long int i;
    for (i=0; i<n; i++)
    {
        vetor[i] = -2000 + (rand() % (4000));
    }
}

double norma (long int n, float *vetor)
{
    long int i;
    double f=0;
    for (i=0; i<n; i++)
    {
        f+= vetor[i]*vetor[i];
    }
    return sqrt(f);
}

double produto_escalar (long int n, float *v1, float *v2)
{
    long int i;
    double f=0;
    for (i=0; i<n; i++)
    {
        f+= v1[i]*v2[i];
    }
    return f;
}

double angulo (long int n, float *v1, float* v2)
{
    double produto, norma_v1, norma_v2, ang;

    produto = produto_escalar(n, v1, v2);
    norma_v1 = norma(n, v1);
    norma_v2 = norma(n, v2);
    ang = produto/(norma_v1*norma_v2);
    return acos(ang);
}

void imprime_vetor(long int n, float *vetor)
{
    long int i;
    for (i=0; i<n; i++)
    {
        printf("%.f ", vetor[i]);
    }
    printf("\n");
}

void papi (long int n, float *v1, float *v2, long int memoria, FILE* arq, double medias[])
{
    int Papi;
    float tempoRealFlops, tempoProcessamentoFlops, megaFlops;//saída
    long long flpops;//saída
    float tempoRealEntradaFlops, tempoProcessamentoEntradaFlops, megaFlopsEntrada; //entrada
    long long iflpops; //entrada

    if((Papi=PAPI_flops(&tempoRealEntradaFlops, &tempoProcessamentoEntradaFlops, &iflpops, &megaFlopsEntrada)) < PAPI_OK)   // inicializa contadores antes do codigo
    {
        printf("Nao foi possivel iniciar PAPI_FLOPS \n");
        exit(1);
    }

    angulo(n, v1, v2);

    if((Papi=PAPI_flops(&tempoRealFlops, &tempoProcessamentoFlops, &flpops, &megaFlops))<PAPI_OK)    // finaliza contadores
    {
        exit(1);
    }

    fprintf(arq, "%f|%f|%lld|%f|%.6lf\n", tempoRealFlops, tempoProcessamentoFlops, flpops, megaFlops/1000, (double) (memoria*2)/tempoRealFlops);
    medias[0]+= (double) (megaFlops/1000);
    medias[1]+= (double) (memoria*2)/tempoRealFlops;

    PAPI_shutdown();
}

void imprimir_media (FILE* arq, long int memoria, double medias[])
{
    fprintf(arq, "%ld|", memoria*2);
    fprintf(arq, "%.6lf|", medias[1]/20);
    fprintf(arq, "%.6lf\n", medias[0]/20);
    medias[0] = 0, medias[1] = 0;
}

int main ()
{
    float *v1, *v2;
    long int memoria = KB, elementos;
    int j;
    double medias[2];
    medias[0] = 0, medias[1] = 0;
    srand((unsigned)time(NULL));
    FILE* arq = fopen("resultados.txt", "w");
    FILE* saida = fopen("medias.txt", "w");

    for (memoria=KB; memoria<=GB; memoria*=2)
    {
        elementos = memoria/sizeof(float);
        v1 = malloc(elementos*sizeof(float));
        v2 = malloc(elementos*sizeof(float));

        for (j=0; j<20; j++)
        {
            gera_elementos(elementos, v1);
            gera_elementos(elementos, v2);
            papi(elementos, v1, v2, memoria, arq, medias);
        }
        printf("memoria %ld elementos %ld completos\n", memoria*2, elementos);
        imprimir_media(saida, memoria, medias);
        free(v1);
        free(v2);
    }

    fclose(arq);
    fclose(saida);
    return 0;
}
