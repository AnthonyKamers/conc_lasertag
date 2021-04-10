#include "prateleira.h"
#include "arranjo.h"
#include "partida.h"

/**
 * ATENÇÃO: Você pode adicionar novas funções com PUBLIC para serem usadas por
 * outros arquivos e adicionar a assinatura da função no .h referente.
 */

/*============================================================================*
 * Definição das variáveis globais (publicas ou privadas)                     *
 *============================================================================*/

/* Adicione variáveis locais. */

arranjo_equipamentos_t prateleira;
int qtdMax;

/*============================================================================*
 * prateleira_setup()                                                         *
 *============================================================================*/

/**
 * @brief Configura os recursos locais se existirem.
 */

// void arranjo_print_prateleira() {

// 	for (int i = 0; i < prateleira_global->size; i++) {
// 		equipamentos_t *equipamento = (equipamentos_t *) prateleira_global->conteudo[i];

// 		if (equipamento != NULL) {
// 			printf("colete = %d | capacete = %d | arma = %d \n", equipamento->colete, equipamento->capacete, equipamento->arma);
// 		}
// 	}
// }

PUBLIC void prateleira_setup(void)
{
	qtdMax = params->jogadores_por_equipe * 2;

	arranjo_iniciar(&prateleira.colete, qtdMax, 0);
	arranjo_iniciar(&prateleira.capacete, qtdMax, 0);
	arranjo_iniciar(&prateleira.arma, qtdMax, 0);

	for (int i = 0; i < qtdMax; i++) {
		arranjo_colocar(&prateleira.colete, (void *) (intptr_t) i);
		arranjo_colocar(&prateleira.capacete, (void *) (intptr_t) i);
		arranjo_colocar(&prateleira.arma, (void *) (intptr_t) i);
	}
}

/*============================================================================*
 * prateleira_cleanup()                                                         *
 *============================================================================*/

/**
 * @brief Limpa os recursos locais se existirem.
 */
PUBLIC void prateleira_cleanup(void)
{
	/**
	 * ATENÇÃO: A quantidade de equipamentos de cada tipo ao final da execução
	 * do programa deve ser igual a quantidade inicial.
	 */

	/* Complemente se precisar. */

	if (
		arranjo_tamanho(&prateleira.capacete) == qtdMax &&
		arranjo_tamanho(&prateleira.colete) == qtdMax &&
		arranjo_tamanho(&prateleira.arma) == qtdMax
	) {
		plog("prateleira está com a mesma quantidade que começou! CERTO \n");
	} else {
		plog("prateleira está errada! ERROUUUUUUUUUUU \n");
	}
}

/*============================================================================*
 * prateleira_pega_equipamentos()                                             *
 *============================================================================*/

/**
 * @brief Pega equipamentos.
 *
 * @return equipamentos Slot de equipamentos de um jogador.
 *
 * Descrição: O jogador deve tentar pegar da prateleira os equipamentos
 * necessários. Especificamente, um de cada tipo: colete, capacete e arma.
 *
 * Os equipamentos devem ter identificadores únicos variando dentro do intervalo:
 * 0 <= ID < 2 * params->jogadores_por_equipe.		---> 
 *
 * ATENÇÃO: Um identificador deve ser único e não pode ser retornado se o mesmo
 * já estiver alocado para um jogador.
 *
 * Dica: Você deve utilizar uma estrutura de dados para armazenar os
 * identificadores dos equipamentos. Para isso, implemente a estrutura "arranjo",
 * cujo esqueleto já foi definido. Veja detalhes de implementação dessa estrutura
 * no arquivo arranjo.h.
 */
PUBLIC void prateleira_pega_equipamentos(equipamentos_t * equipamentos)
{
	assert(equipamentos);

	int random_colete = aleatorio(0, arranjo_tamanho(&prateleira.colete));
	int random_capacete = aleatorio(0, arranjo_tamanho(&prateleira.capacete));
	int random_arma = aleatorio(0, arranjo_tamanho(&prateleira.arma));

	int colete_id = (int) (intptr_t) arranjo_pop(&prateleira.colete, random_colete);
	int capacete_id = (int) (intptr_t) arranjo_pop(&prateleira.capacete, random_capacete);
	int arma_id = (int) (intptr_t) arranjo_pop(&prateleira.arma, random_arma);

	equipamentos->colete = colete_id;
	equipamentos->capacete = capacete_id;
	equipamentos->arma = arma_id;
}

/*============================================================================*
 * prateleira_libera_equipamentos()                                           *
 *============================================================================*/

/**
 * @brief Libera equipamentos após sua limpeza.
 *
 * @return equipamentos Slot de equipamentos limpos.
 *
 * Descrição: O limpador deve armazenar novamente os equipamentos que acabaram
 * de serem limpos.
 *
 * ATENÇÃO: Um identificador deve ser único e não pode ser retornado se o mesmo
 * já estiver alocado para um jogador.
 */
PUBLIC void prateleira_libera_equipamentos(equipamentos_t * equipamentos)
{
	assert(equipamentos);

	arranjo_colocar(&prateleira.colete, (void *) (intptr_t) equipamentos->colete);
	arranjo_colocar(&prateleira.capacete, (void *) (intptr_t) equipamentos->capacete);
	arranjo_colocar(&prateleira.arma, (void *) (intptr_t) equipamentos->arma);

	sem_post(&partida->semaforo_equipamentos_disponiveis);
}

