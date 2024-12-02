#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "libreria.h"
long int num_registros(char *file_name)
{
	FILE *fp;
	int nc, ncl, flag;
	char c;
	long int nr;
	fp = fopen(file_name, "rt");
	if(fp==NULL)
		return 0;
	fflush(fp);
	nc=0;
	do{
		c = getc(fp);
		if(c==',')
			nc++;
	}while(c!='\n');
	c = getc(fp);
	ncl = 0;
	nr = 0;
	flag = 1;
	while(c!=EOF)
	{
		if((c==',')&flag)
			ncl++;
		if(c=='"')
			flag = !flag;
		if(c=='\n')
		{
			if(ncl==nc)
				nr++;
			ncl = 0;
		}
		c = getc(fp);
	}
	fclose(fp);
	return  nr;
}

int lectura_bd(char *file_name, bd_INEGI *Datos)
{
	int nc, *realloc_nid;
	long int nr, i, j, k, l, flag, np, *realloc_id, **realloc_id_d;
	char **palabra;
	float fct;
	char buffer[MAX_STR], c, **realloc_dic;
	estados estado[9] = {conv_entero, cadena_s, conv_entero, cadena_d, 
		conv_enterol, cadena_i, conv_entero, conv_enterol, sincronizacion};
	FILE *fp;
	nr = num_registros(file_name);
	if(nr==0)
		return 1;
	fp = fopen(file_name, "rt");
	if(fp==NULL)
		return 2;
	ini_bd(Datos);
	Datos->nr = nr;
	Datos->mem = (int*)malloc(3*nr*sizeof(int)+2*nr*sizeof(long int));
	if(Datos->mem==NULL)
	{
		fclose(fp);
		return 3;
	}
	fct = (1.0*sizeof(long int))/sizeof(int);
	Datos->cve_entidad=Datos->mem;
	Datos->mem+=nr;
	Datos->cve_municipio=Datos->mem;
	Datos->mem+=nr;
	Datos->id_indicador=(long int*)(Datos->mem);
	Datos->mem+=((int)(fct*nr));
	Datos->anio=Datos->mem;
	Datos->mem+=nr;
	Datos->valor=(long int*)(Datos->mem);
	Datos->mem=Datos->cve_entidad;
	Datos->desc_entidad.palabra = (char**)malloc(nr*sizeof(char*));
	if(Datos->desc_entidad.palabra==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 4;
	}
	Datos->desc_entidad.np = 0;
	Datos->desc_municipio.palabra = (char**)malloc(nr*sizeof(char*));
	if(Datos->desc_municipio.palabra==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 5;
	}
	Datos->desc_municipio.np = 0;
	Datos->desc_municipio.id = (long int**)malloc(nr*sizeof(long int*));
	if(Datos->desc_municipio.id==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 6;
	}
	Datos->desc_municipio.ntid = 0;
	Datos->desc_municipio.nid = (int*)calloc(nr,sizeof(int));
	if(Datos->desc_municipio.nid==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 7;
	}
	Datos->indicador.palabra = (char**)malloc(nr*sizeof(char*));
	if(Datos->indicador.palabra==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 8;
	}
	Datos->indicador.id = (long int*)malloc(nr*sizeof(long int));
	if(Datos->indicador.id==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 9;
	}
	Datos->indicador.np = 0;
	printf("Numero de registros: %ld\n", Datos->nr);
	do{
		c=getc(fp);
	}while(c!='\n');
	for(i=0; i<Datos->nr; i++)
	{
		Datos->mem = Datos->cve_entidad;
		for(k=0; k<9; k++)
		{
			j = 0;
			flag = 0;
			do{
				buffer[j++] = getc(fp);
				if(buffer[j-1]=='"')
					flag = !flag;
			}while(flag||(buffer[j-1]!=(estado[k]==sincronizacion?'\n':',')));
			buffer[j-1] = '\0';
			nc = strlen(buffer);
			switch(estado[k])
			{
			case conv_entero:
				Datos->mem[i] = atoi(buffer);
				Datos->mem+=nr;
				break;
			case conv_enterol:
				Datos->mem[(int)(fct*i)] = atol(buffer);
				Datos->mem+=(int)(fct*nr);
				break;
			case cadena_s:
			case cadena_i:
				np = estado[k]==cadena_s?Datos->desc_entidad.np:Datos->indicador.np;
				palabra = estado[k]==cadena_s?Datos->desc_entidad.palabra:Datos->indicador.palabra;
				flag = !np;
				if(np)
				{
					j=0;
					while(strcmp(palabra[j++], buffer))
						if(j==np)
						{
							flag=1;
							break;
						}
				}
				if(flag)
				{
					palabra[np] = (char*)malloc((nc+1)*sizeof(char));
					if(palabra[np]==NULL)
					{
						for(j=0; j<np; j++)
							free(palabra[j]);
						liberar_mem(Datos);
						fclose(fp);
						return 10;
					}
					strcpy(palabra[np], buffer);
					if(estado[k]==cadena_i)
						Datos->indicador.id[np] = Datos->id_indicador[i];
					estado[k]==cadena_s?Datos->desc_entidad.np++:Datos->indicador.np++;
				}
				break;
			case cadena_d:
				if(Datos->cve_entidad[i]==36)
					l = 33;
				else if(Datos->cve_entidad[i]==99)
					l = 34;
				else
					l = Datos->cve_entidad[i];
				flag = !(Datos->desc_municipio.np);
				j=0;
				while((!flag)&&strcmp(Datos->desc_municipio.palabra[j++], buffer))
					if(j==Datos->desc_municipio.np)
						flag = 1;
				if(flag)
				{
					Datos->desc_municipio.palabra[j] = (char*)malloc((nc+1)*sizeof(char));
					if(Datos->desc_municipio.palabra[j]==NULL)
					{
						liberar_mem(Datos);
						fclose(fp);
						return 11;
					}
					strcpy(Datos->desc_municipio.palabra[j], buffer);
					Datos->desc_municipio.np++;
				}
				if(Datos->desc_municipio.ntid==l)
				{
					Datos->desc_municipio.id[Datos->desc_municipio.ntid] = (long int *)malloc(NM_M*sizeof(long int));
					if(Datos->desc_municipio.id[Datos->desc_municipio.ntid]==NULL)
					{
						liberar_mem(Datos);
						return 12;
					}
					Datos->desc_municipio.ntid++;
				}
				if(Datos->desc_municipio.nid[l]==Datos->cve_municipio[i])
				{
					Datos->desc_municipio.nid[l]++;
					Datos->desc_municipio.id[l][Datos->cve_municipio[i]] = flag?j:j-1;
				}
				break;
			default:
				break;
			}
		}
	}
	realloc_dic = (char**)realloc(Datos->desc_entidad.palabra, (Datos->desc_entidad.np)*sizeof(char*));
	if(realloc_dic==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 13;
	}
	else
		Datos->desc_entidad.palabra = realloc_dic;
	realloc_id_d = (long int**)realloc(Datos->desc_municipio.id, (Datos->desc_municipio.ntid)*sizeof(long int*));
	if(realloc_id_d==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 14;
	}
	else
		Datos->desc_municipio.id = realloc_id_d;
	for(i=0; i<Datos->desc_municipio.ntid; i++)
	{
			realloc_id = (long int*)realloc(Datos->desc_municipio.id[i], (Datos->desc_municipio.nid[i])*sizeof(long int));
			if(realloc_id==NULL)
			{
				liberar_mem(Datos);
				fclose(fp);
				return 19+i;
			}
			else
				Datos->desc_municipio.id[i] = realloc_id;
	}
	realloc_nid = (int*)realloc(Datos->desc_municipio.nid, (Datos->desc_municipio.ntid)*sizeof(int));
	if(realloc_nid==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 15;
	}
	else
		Datos->desc_municipio.nid = realloc_nid;
	realloc_dic = (char**)realloc(Datos->desc_municipio.palabra, (Datos->desc_municipio.np)*sizeof(char*));
	if(realloc_dic==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 16;
	}
	else
		Datos->desc_municipio.palabra = realloc_dic;
	realloc_dic = (char**)realloc(Datos->indicador.palabra, (Datos->indicador.np)*sizeof(char*));
	if(realloc_dic==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 17;
	}
	else
		Datos->indicador.palabra = realloc_dic;
	realloc_id = (long int*)realloc(Datos->indicador.id, (Datos->indicador.np)*sizeof(long int));
	if(realloc_id==NULL)
	{
		liberar_mem(Datos);
		fclose(fp);
		return 18;
	}
	else
		Datos->indicador.id = realloc_id;
	fclose(fp);
	return 0;
}

char* buscar(diccionario_i dic, long int id)
{
	long int i;
	i = 0;
	while(dic.id[i++]!=id)
		if(i==dic.np)
			return NULL;
	return dic.palabra[i-1];
}

int buscar_i(diccionario_r dic, char *palabra)
{
	long int i;
	i = 0;
	while(strcmp(dic.palabra[i++], palabra))
		if(i==dic.np)
			return -1;
	return i-1;
}

void imprimir_dr(diccionario_r dic)
{
	long int i;
	printf("%ld\n", dic.np);
	for(i=0; i<dic.np; i++)
		printf("%ld. %s\n", i, dic.palabra[i]);
}

void imprimir_di(diccionario_i dic)
{
	long int i;
	printf("%ld\n", dic.np);
	for(i=0; i<dic.np; i++)
		printf("%ld. %ld\t%s\n", i, dic.id[i], dic.palabra[i]);
}

void imprimir_dd(diccionario_d dic)
{
	long int i, j;
	printf("%ld\n", dic.np);
	for(i=0; i<dic.np; i++)
		printf("%ld. %s\n", i, dic.palabra[i]);
	printf("%ld\n", dic.ntid);
	for(i=0; i<dic.ntid; i++)
	{
		printf("%ld. %d\t", i, dic.nid[i]);
		for(j=0; j<dic.nid[i]; j++)
			printf("%ld ", dic.id[i][j]);
		printf("\n");
	}
}

int liberar_bd(bd_INEGI Datos)
{
	long int i;
	free(Datos.desc_municipio.nid);
	for(i=0; i<Datos.desc_municipio.ntid; i++)
		free(Datos.desc_municipio.id[i]);
	free(Datos.desc_municipio.id);
	for(i=0; i<Datos.desc_municipio.np; i++)
		free(Datos.desc_municipio.palabra[i]);
	free(Datos.desc_municipio.palabra);
	free(Datos.cve_entidad);
	for(i=0; i<Datos.desc_entidad.np; i++)
		free(Datos.desc_entidad.palabra[i]);
	free(Datos.desc_entidad.palabra);
	for(i=0; i<Datos.indicador.np; i++)
		free(Datos.indicador.palabra[i]);
	free(Datos.indicador.palabra);
	free(Datos.indicador.id);
	return 0;
}

int ini_bd(bd_INEGI *Datos)
{
	Datos->nr = 0;
	Datos->mem = NULL;
	Datos->cve_entidad = NULL;
	Datos->desc_entidad.palabra = NULL;
	Datos->desc_entidad.np = 0;
	Datos->cve_municipio = NULL;
	Datos->desc_municipio.id = NULL;
	Datos->desc_municipio.nid = NULL;
	Datos->desc_municipio.ntid = 0;
	Datos->desc_municipio.palabra = NULL;
	Datos->desc_municipio.np = 0;
	Datos->id_indicador = NULL;
	Datos->indicador.id = NULL;
	Datos->indicador.palabra = NULL;
	Datos->indicador.np = 0;
	Datos->anio = NULL;
	Datos->valor = NULL;
	return 0;
}

int liberar_mem(bd_INEGI *Datos)
{
	long int i;
	if(Datos->cve_entidad != NULL)
		free(Datos->cve_entidad);
	Datos->nr = 0;
	Datos->mem = NULL;
	Datos->cve_entidad = NULL;
	Datos->cve_municipio = NULL;
	Datos->id_indicador = NULL;
	Datos->anio = NULL;
	Datos->valor = NULL;
	if(Datos->desc_entidad.palabra!=NULL)
	{
		for(i=0; i<Datos->desc_entidad.np; i++)
			free(Datos->desc_entidad.palabra[i]);
		free(Datos->desc_entidad.palabra);
		Datos->desc_entidad.palabra = NULL;
		Datos->desc_entidad.np = 0;
	}
	if(Datos->desc_municipio.palabra!=NULL)
	{
		for(i=0; i<Datos->desc_municipio.np; i++)
			free(Datos->desc_municipio.palabra[i]);
		free(Datos->desc_municipio.palabra);
		Datos->desc_municipio.palabra = NULL;
		Datos->desc_municipio.np = 0;
	}
	if(Datos->desc_municipio.id!=NULL)
	{
		for(i=0; i<Datos->desc_municipio.ntid; i++)
			free(Datos->desc_municipio.id[i]);
		free(Datos->desc_municipio.id);
		Datos->desc_municipio.id = NULL;
		Datos->desc_municipio.ntid = 0;
	}
	if(Datos->desc_municipio.nid!=NULL)
	{
		free(Datos->desc_municipio.nid);
		Datos->desc_municipio.nid = NULL;
	}
	if(Datos->indicador.palabra!=NULL)
	{
		for(i=0; i<Datos->indicador.np; i++)
			free(Datos->indicador.palabra[i]);
		free(Datos->indicador.palabra);
		Datos->indicador.palabra = NULL;
		Datos->indicador.np = 0;
	}
	if(Datos->indicador.id!=NULL)
	{
		free(Datos->indicador.id);
		Datos->indicador.id = NULL;
	}
	return 0;
}
//Encuentra el primer registro de data que tiene el cve_entidad pasado a la funcion
long int encontrar_registro_cve_entidad(long int cve_entidad, bd_INEGI* Data)
{
	//set i (registry we will check) to the middle point by diving the total number of registries into 2
	long int i = ((Data->nr)/2);
	//we set bottom at 0 since its from where we first calculate the middle and the same with top being nr the max point
	long int bottom = 0,top = Data->nr;
	//we do everything until the current value is equal to the target value
	while(Data->cve_entidad[i] != cve_entidad)
	{
		if(cve_entidad<Data->cve_entidad[i])
		{
			//if our target value is under our current value we set the maximum point to the current value
			top = i;
		}
		else if(cve_entidad>Data->cve_entidad[i])
		{
			//if our target value is over our current value we set the minimum point to the current value
			bottom = i;
		}
		//we set a new anchor point between the minimum and maximum point 
		i = (bottom+top)/2;
	}
	//we find the first item containing the target value by checking if the previous one is not the same
	while(Data->cve_entidad[i] == cve_entidad)
		i--;
	//we add one to the value since the last value was equal but the current is not
	i++;
	//we return the iterator which is the registry number of the first coincidence
	return i;
}
long int cantidad_total_defunciones(long int reg,bd_INEGI Datos){
    long int municipios_defunciones=0;
    //municipios_defunciones=(long int*)malloc(sizeof(Datos.cve_municipio)*sizeof(long int));
    while (!Datos.cve_municipio[reg])
        reg++;
    //Tiene dos de diferencia
    long int municipio_mayor=0;
    long int reg_mun=0;
    int id_mayor=0;
    int id=Datos.cve_municipio[reg];
    while (id!=996){
        municipios_defunciones=0;
        while (Datos.id_indicador[reg]==1002000030){
            municipios_defunciones+=Datos.valor[reg];
            reg++;
        }
        if(municipio_mayor<municipios_defunciones){
            id_mayor=id;
            reg_mun= Datos.desc_municipio.id[Datos.cve_entidad[reg]][Datos.cve_municipio[reg]];
            municipio_mayor=municipios_defunciones;
        }
        reg++;
        id=Datos.cve_municipio[reg];
    }
    printf("Def: %ld\nMayor municipio (%s)   (%d)",municipio_mayor,Datos.desc_municipio.palabra[reg_mun],id_mayor);
    return municipios_defunciones;
}
