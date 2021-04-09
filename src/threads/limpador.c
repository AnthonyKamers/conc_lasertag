#include "config.h"
#include "prateleira.h"
#include "partida.h"

/**
 * ATENÇÃO: Você pode adicionar novas funções com PUBLIC para serem usadas por
 * outros arquivos e adicionar a assinatura da função no .h referente.
 */

/*============================================================================*
 * Definição das variáveis globais (publicas ou privadas)                     *
 *============================================================================*/

/*============================================================================*
 * limpador_setup()                                                           *
 *============================================================================*/

/**
 * @brief Configura os recursos locais do limpador caso existam.
 */
PUBLIC void limpador_setup(void)
{
}

/*============================================================================*
 * limpador_cleanup()                                                         *
 *============================================================================*/

/**
 * @brief Limpa os recursos locais do limpador caso existam.
 */
PUBLIC void limpador_cleanup(void)
{
}

/*============================================================================*
 * limpador_requisita_limpeza()                                               *
 *============================================================================*/

/**
 * @brief Requisita a limpeza de equipamentos.
 *
 * @param equipamentos Equipamentos para limpeza.
 *
 * Descrição: Os equipamentos devem ser passados para o limpador nesta função.
 */
PUBLIC void limpador_requisita_limpeza(equipamentos_t * equipamentos)
{
	assert(equipamentos);

	/* Requisita limpeza. */
	// salvo para chamar prateleira_libera_equipamento depois
	equipamentos_t *equipamentos_hold = malloc(sizeof(equipamentos_t));
	equipamentos_hold->arma = equipamentos->arma;
	equipamentos_hold->capacete = equipamentos->capacete;
	equipamentos_hold->colete = equipamentos->colete;

	/* Tira equipamentos do jogador. */
	equipamentos->arma     = -1;
	equipamentos->capacete = -1;
	equipamentos->colete   = -1;

	// espera delay_limpador
	msleep(params->delay_limpador);

	// chama função de prateleira para devolver à prateleira 
	prateleira_libera_equipamentos(equipamentos_hold);
}

/*============================================================================*
 * Responsabilidades do Limpador.                                             *
 *============================================================================*/

/**
 * @brief Main do limpador.
 *
 * Descrição: O limpador deve esperar equipamentos para limpar enquanto
 * houverem partidas e jogadores. Quando seu trabalho terminar, o mesmo deve
 * sair.
 */
PUBLIC void * limpador_fn(void * arg)
{
	// deixar thread ativa enquanto houver tempo em tempo_partida
	// ou seja, enquanto tempo_partida < partida_tempo_max e
	// houver jogadores vivos na partida
	while (partida->partida_now <= params->partidas_max) {}

	return (NULL);
}

