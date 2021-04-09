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
arranjo_t *prateleira_global;
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
	prateleira_global = malloc(sizeof(arranjo_t));
	qtdMax = params->jogadores_por_equipe * 2;

	arranjo_iniciar(prateleira_global, qtdMax, 0);

	for (int i = 0; i < qtdMax; i++) {
		int id_now = i;

		equipamentos_t *equipamentos_now = malloc(sizeof(equipamentos_t));
		equipamentos_now->colete = id_now;
		equipamentos_now->capacete = id_now;
		equipamentos_now->arma = id_now;

		/* criar arranjos diferentes para cada equipamento (colete, capacete, arma) */
		/* lista de inteiros */

		arranjo_colocar(prateleira_global, (void *) equipamentos_now);
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
	
	if (arranjo_tamanho(prateleira_global) == qtdMax) {
		plog("prateleira está com todos os itens \n");
	} else {
		plog("prateleira não está com todos os itens ---> falta coisa \n");
	}

	arranjo_destruir(prateleira_global);

	/* Complemente se precisar. */
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

	if (!arranjo_vazio(prateleira_global)) {
		equipamentos_t *equipamento_da_prateleira = (equipamentos_t *) arranjo_retirar(prateleira_global);

		equipamentos->colete = equipamento_da_prateleira->colete;
		equipamentos->capacete = equipamento_da_prateleira->capacete;
		equipamentos->arma = equipamento_da_prateleira->arma;

		free(equipamento_da_prateleira);
	} else {
		plog("a prateleira está vazia, não pega mais daqui, não \n");
	}
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

	if (!arranjo_cheio(prateleira_global)) {
		int is_wrong = 0;

		/*============================================================================*/
		// fazer verificação de toda a prateleira (para ver se não está com o mesmo ID)
		for (int i = 0; i < prateleira_global->size; i++) {
			equipamentos_t *teste = (equipamentos_t *) prateleira_global->conteudo[i];

			if (
				teste->arma == equipamentos->arma ||
				teste->capacete == equipamentos->capacete ||
				teste->colete == equipamentos->colete
			) {
				plog("está com o mesmo ID, não vamos adicionar na prateleira \n");
				is_wrong = 1;
				break;
			}
		}

		// se já não tiver com o mesmo ID na prateleira,
		// adiciona novamente na prateleira e adiciona no semáforo de equipamentos disponíveis
		if (!is_wrong) {
			arranjo_colocar(prateleira_global, equipamentos);
			sem_post(&partida->semaforo_equipamentos_disponiveis);
		}

	} else {
		plog("prateleira já está cheia!! não coloque mais coisas aqui \n");
	}
}

