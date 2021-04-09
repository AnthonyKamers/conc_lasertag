#include "arranjo.h"

/**
 * ATENÇÃO: Você pode adicionar novas funções com PUBLIC para serem usadas por
 * outros arquivos e adicionar a assinatura da função no .h referente.
 */

/*============================================================================*
 * arranjo_iniciar()                                                          *
 *============================================================================*/

/**
 * @brief Inicia e aloca os recursos de um arranjo.
 *
 * @param arranjo    Ponteiro de um arranjo.
 * @param capacidade Tamanho do arranjo.
 */
PUBLIC void arranjo_iniciar(arranjo_t * arranjo, int capacidade, int inicial_semaforo)
{
	sem_init(&(arranjo->semaforo), 0, inicial_semaforo);

	// arranjo = (arranjo_t *) malloc(sizeof(arranjo_t));
	arranjo->capacidade = capacidade;
	arranjo->size = 0;
	arranjo->conteudo = malloc(sizeof(void *) * capacidade);

}

/*============================================================================*
 * arranjo_destruir()                                                         *
 *============================================================================*/

/**
 * @brief Libera recursos de um arranjo.
 *
 * @param arranjo Ponteiro de um arranjo.
 */
PUBLIC void arranjo_destruir(arranjo_t * arranjo)
{
	for (int i = 0; i < arranjo->size; i++) {
		if (arranjo->conteudo[i] != NULL) {
			free(arranjo->conteudo[i]);
		}
	}
	
	if (arranjo->conteudo != NULL) {
		free(arranjo->conteudo);
	}
}

/*============================================================================*
 * arranjo_colocar()                                                          *
 *============================================================================*/

/**
 * @brief Coloca um elemento no arranjo.
 *
 * @param arranjo  Ponteiro de um arranjo.
 * @param elemento Elemento a ser armazenado no arranjo.
 */
PUBLIC void arranjo_colocar(arranjo_t * arranjo, void * elemento) // push_back
{
	if (arranjo_cheio(arranjo)) {
		printf("tentou adicionar no arranjo, mas está cheio \n");
	} else {
		arranjo->conteudo[arranjo->size] = elemento;
		arranjo->size++;
	}
}

/*============================================================================*
 * arranjo_retirar()                                                          *
 *============================================================================*/

/**
 * @brief Retira um elemento do arranjo.
 *
 * @param arranjo Ponteiro de um arranjo.
 *
 * @return Se o arranjo estiver vazio retorna NULL. Caso contrário, retorna
 * um elemento do arranjo.
 */
PUBLIC void * arranjo_retirar(arranjo_t * arranjo) // pop_back
{
	void * elemento = NULL;

	if (arranjo_vazio(arranjo)) {
		printf("tentou retirar do arranjo, mas está vazio");
	} else {
		elemento = arranjo->conteudo[arranjo->size - 1];
		arranjo->size--;
	}

    return elemento;
}

/*============================================================================*
 * arranjo_pop()                                                          *
 *============================================================================*/
PUBLIC void * arranjo_pop(arranjo_t * arranjo, int index) { // pop
	void * elemento = NULL;

	if (index > 0 && index < arranjo->size) {
		elemento = arranjo->conteudo[index];
		arranjo->size--;

		for (int i = index; i < arranjo->size; i++) {
			arranjo->conteudo[i] = arranjo->conteudo[i + 1];
		}
	}

	return elemento;
}

/*============================================================================*
 * arranjo_remover()                                                          *
 *============================================================================*/

/**
 * @brief Remove um elemento do arranjo.
 *
 * @param arranjo  Ponteiro de um arranjo.
 * @param valor    Elemento a ser removido.
 */
PUBLIC void arranjo_remover(arranjo_t * arranjo, void * elemento) // remove
{
	if (arranjo_vazio(arranjo)) {
		printf("tentou remover do arranjo, mas já está vazio");
	} else {
		// loop pelo arranjo
		for (int i = 0; i < arranjo->capacidade; i++) {
			if (arranjo->conteudo[i] == elemento) {
				arranjo_pop(arranjo, i);
				break;
			}
		}
	}
}

/*============================================================================*
 * arranjo_vazio()                                                            *
 *============================================================================*/

/**
 * @brief Verifica se o arranjo está vazio.
 *
 * @param arranjo Ponteiro de um arranjo.
 *
 * @return Diferente de zero se o arranjo está vazio. Zero, caso contrário.
 */
PUBLIC int arranjo_vazio(arranjo_t * arranjo)
{
	return arranjo->size == 0 ? true : false;
}

// arranjo cheio
PUBLIC int arranjo_cheio(arranjo_t *arranjo) {
	return arranjo->size == arranjo->capacidade;
}

/*============================================================================*
 * arranjo_tamanho()                                                          *
 *============================================================================*/

/**
 * @brief Verifica o tamanho do arranjo.
 *
 * @param arranjo Ponteiro de um arranjo.
 *
 * @return Retorna o tamanho atual do arranjo.
 */
PUBLIC int arranjo_tamanho(arranjo_t * arranjo)
{
	return arranjo->size;
}

