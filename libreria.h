#ifndef LIBRERIA_H
#define LIBRERIA_H

#define MAX_STR 200
#define NM_M 600
typedef union dato{
	float f;
	int i;
	char c[4];
}dato;
typedef struct diccionario_r{
	char **palabra;
	long int np;
}diccionario_r;

typedef struct diccionario_i{
	long int *id;
	char **palabra;
	long int np;
}diccionario_i;

typedef struct diccionario_d{
	long int **id;
	int *nid;
	long int ntid;
	char **palabra;
	long int np;
}diccionario_d;

typedef struct bd_INEGI{
	long int nr;
	int *mem;
	int *cve_entidad;
	diccionario_r desc_entidad;
	int *cve_municipio;
	diccionario_d desc_municipio;
	long int *id_indicador;
	diccionario_i indicador;
	int *anio;
	long int *valor;
}bd_INEGI;

typedef enum estados{
	separador, conv_entero, conv_enterol, cadena_s, cadena_i, cadena_d, sincronizacion
}estados;

long int num_registros(char *file_name);
int lectura_bd(char *file_name, bd_INEGI *Datos);
char* buscar(diccionario_i dic, long int id);
int buscar_i(diccionario_r dic, char *palabra);
int liberar_bd(bd_INEGI Datos);
void imprimir_dr(diccionario_r dic);
void imprimir_di(diccionario_i dic);
void imprimir_dd(diccionario_d dic);
int liberar_mem(bd_INEGI *Datos);
int ini_bd(bd_INEGI *Datos);

#endif