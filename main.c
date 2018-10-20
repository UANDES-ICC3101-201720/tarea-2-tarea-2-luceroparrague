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

//nodo base
typedef struct node {
    int pagina;
		int marco;
    struct node * next;
} node_t;

//agregar nodo al final de la lista
void push_l(node_t * head, int pagina) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(node_t));
    current->next->pagina = pagina;
    current->next->next = NULL;
}

//para dejar marcada la variable del marco tmb
void push_l_m(node_t * head, int pagina, int marco) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(node_t));
    current->next->pagina = pagina;
		current->next->marco = marco;
    current->next->next = NULL;
}

//agregar nodo al principio de la lista
//push_i(&lista,numero);
void push_i(node_t ** head, int pagina) {
    node_t * new_node;
    new_node = malloc(sizeof(node_t));

    new_node->pagina = pagina;
    new_node->next = *head;
    *head = new_node;
}

//lo mismo pero tmb editar la variable marco
void push_i_m(node_t ** head, int pagina,int marco) {
    node_t * new_node;
    new_node = malloc(sizeof(node_t));

    new_node->pagina = pagina;
		new_node->marco = marco;
    new_node->next = *head;
    *head = new_node;
}

//eliminar el primer nodo de una lista ligada
int pop_i(node_t ** head) {
    int retpagina = -1;
    node_t * next_node = NULL;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    retpagina = (*head)->pagina;
    free(*head);
    *head = next_node;

    return retpagina;
}

//eliminar el ultimo nodo de una lista
int pop_l(node_t * head) {
    int retpagina = 0;
    /* if there is only one item in the list, remove it */
    if (head->next == NULL) {
        retpagina = head->pagina;
        free(head);
        return retpagina;
    }

    /* get to the second to last node in the list */
    node_t * current = head;
    while (current->next->next != NULL) {
        current = current->next;
    }

    /* now current points to the second to last item of the list, so let's remove current->next */
    retpagina = current->next->pagina;
    free(current->next);
    current->next = NULL;
    return retpagina;
}

//elimina un nodo especifico de una lista
int pop_index(node_t ** head, int n) {
    int i = 0;
    int retpagina = -1;
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
    retpagina = temp_node->pagina;
    current->next = temp_node->next;
    free(temp_node);

    return retpagina;
}

// imprime linked list
void print_list(node_t * head) {
    node_t * current = head;

    while (current != NULL) {
        printf("%d\n", current->pagina);
				printf("marco %i\n", current->marco);
        current = current->next;
    }
}

//aqui termino comandos linked list

node_t *stack = NULL;

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


//este lo estoy usando para testear cosas
void page_fault_handler_nuestro(struct page_table *pt, int page){
	printf("handler nuestro page fault on page  #%d\n",page);

	//"Si la aplicación comienza intentando leer la página 2, esto causará una falta de página.""
	//"El manejador de falta de página escogerá un marco libre, por ejemplo, el 3.""
	node_t * nodo = stack;
	int marco_a_usar = -1;
	while (nodo != NULL && marco_a_usar == -1){
		if (nodo->pagina == -1){
			marco_a_usar = nodo->marco;
			nodo->pagina = page;
		}
		nodo = nodo->next;
	}
	printf("marco a utilizar %i\n",marco_a_usar);

	if (marco_a_usar != -1){
		page_table_set_entry(pt,page,marco_a_usar,PROT_READ|PROT_WRITE);
		//Finalmente, cargará la página 2 desde el disco al marco 3 (¿¿¿Como???)
		char * puntero;
		puntero = page_table_get_physmem(pt);
		disk_read(disco,page, &puntero[marco_a_usar * BLOCK_SIZE]);
	}
	//page_table_print(pt);
	print_list(stack);

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

	//node_t stack = NULL;
	stack = malloc(sizeof(node_t));
	stack->pagina = -1;
	stack->next = NULL;
	stack->marco = 0;

	for (int i=1; i < nframes; i++){
		push_l_m(stack,-1,i);
	}
	//print_list(stack);


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
