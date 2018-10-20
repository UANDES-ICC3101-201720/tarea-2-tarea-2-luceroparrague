/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//variables globales

//esta tabla mantiene el estado de las memorias fisicas, 0 es libres 1 es ocupada
int * tabla_frames;
int largo_tabla_frames;

//para acceder al disk dentro de los handlers
struct disk *disco;

//Aqui inicio comandos linked list

typedef struct node {
    int val;
    struct node * next;
} node_t;

//agregar nodo al final de la lista
void push_fin(node_t * head, int val) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(node_t));
    current->next->val = val;
    current->next->next = NULL;
}

//agregar nodo al principio de la lista
void push_i(node_t ** head, int val) {
    node_t * new_node;
    new_node = malloc(sizeof(node_t));

    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}

//eliminar el primer nodo de una lista ligada
int pop_i(node_t ** head) {
    int retval = -1;
    node_t * next_node = NULL;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->val;
    free(*head);
    *head = next_node;

    return retval;
}

//eliminar el ultimo nodo de una lista
int pop_l(node_t * head) {
    int retval = 0;
    /* if there is only one item in the list, remove it */
    if (head->next == NULL) {
        retval = head->val;
        free(head);
        return retval;
    }

    /* get to the second to last node in the list */
    node_t * current = head;
    while (current->next->next != NULL) {
        current = current->next;
    }

    /* now current points to the second to last item of the list, so let's remove current->next */
    retval = current->next->val;
    free(current->next);
    current->next = NULL;
    return retval;
}

//elimina un nodo especifico de una lista
int pop_index(node_t ** head, int n) {
    int i = 0;
    int retval = -1;
    node_t * current = *head;
    node_t * temp_node = NULL;

    if (n == 0) {
        return pop_i(head);
    }

    for (i = 0; i < n-1; i++) {
        if (current->next == NULL) {
            return -1;
        }
        current = current->next;
    }

    temp_node = current->next;
    retval = temp_node->val;
    current->next = temp_node->next;
    free(temp_node);

    return retval;
}
//aqui termino comandos linked list


void page_fault_handler_default( struct page_table *pt, int page )
{
	printf("page fault on page #%d\n",page);
	exit(1);
}

void page_fault_handler_fifo( struct page_table *pt, int page )
{
	printf("handler fifo page fault on page #%d\n",page);
	exit(1);
}

void page_fault_handler_rand( struct page_table *pt, int page )
{
	printf("handler rand page fault on page #%d\n",page);
	exit(1);
}

void page_fault_handler_nuestro( struct page_table *pt, int page )
{
	/* //esto fue estudio personal
	page_table_set_entry(pt,2,0,PROT_WRITE|PROT_READ);
	page_table_print_entry(pt,2);
	page_table_print(pt);
	printf("%i\n",page_table_get_nframes(pt));
	printf("%i\n", page_table_get_npages(pt));
	*/
	printf("handler nuestro page fault on page  #%d\n",page);
	//"Si la aplicación comienza intentando leer la página 2, esto causará una falta de página.""
	//"El manejador de falta de página escogerá un marco libre, por ejemplo, el 3.""
	int i = -1;
	for (int a = 0; a < largo_tabla_frames; ++a){
		if (tabla_frames[a] == 0){
				i = a;
				tabla_frames[i] = 1;
				a = largo_tabla_frames;
		}
	}


	if (i != -1){
		page_table_set_entry(pt,page,i,PROT_READ|PROT_WRITE); //Luego ajustará la tabla de páginas para que la página 2 quede asociada al marco 3, con permisos de lectura.

		//Finalmente, cargará la página 2 desde el disco al marco 3 (¿¿¿Como???)
		char * puntero;
		puntero = page_table_get_physmem(pt);
		disk_read(disco,page, &puntero[i * BLOCK_SIZE]);
		printf("%i\n",i);
	}


	page_table_print(pt);

	//exit(1);
}


int main( int argc, char *argv[] )
{
	if(argc!=5) {
		/* Add 'random' replacement algorithm if the size of your group is 3 */
		printf("use: virtmem <npages> <nframes> <lru|fifo> <sort|scan|focus>\n");
		return 1;
	}
	int npages = atoi(argv[1]);

	int nframes = atoi(argv[2]);
	largo_tabla_frames = nframes;

	const char *program = argv[4];

	char *algoritmo = argv[3];

	struct disk *disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}
	disco = disk;


	//esta tabla mantiene el estado de las memorias fisicas, 0 es libres 1 es ocupada
	int frame_table[nframes];
	for (int i = 0; i < nframes; ++i){
			frame_table[i] = 0;
	}
	tabla_frames = frame_table;



		/* codigo default
		struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
		if(!pt) {
			fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
			return 1;
		}
		*/

	struct page_table *pt;
	if (strcmp(algoritmo,"rand") == 0){

		pt = page_table_create( npages, nframes, page_fault_handler_rand );
		if(!pt) {
			fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
			return 1;
		}
	}
	else if (strcmp(algoritmo,"fifo") == 0){
		pt = page_table_create( npages, nframes, page_fault_handler_fifo );
		if(!pt) {
			fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
			return 1;
		}
	}
	else if (strcmp(algoritmo,"our") == 0){
		//printf("el if funciona\n");
		pt = page_table_create( npages, nframes, page_fault_handler_nuestro );
		if(!pt) {
			fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
			return 1;
		}
	}
	else {
		pt = page_table_create( npages, nframes, page_fault_handler_default );
		if(!pt) {
			fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
			return 1;
		}
	}

	char *virtmem = page_table_get_virtmem(pt);

	char *physmem = page_table_get_physmem(pt);


	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);}



	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
