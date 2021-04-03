#include "partida.h"
#include "jogador.h"

/**
 * ATENÇÃO: Você pode adicionar novas funções com PUBLIC para serem usadas por
 * outros arquivos e adicionar a assinatura da função no .h referente.
 */

/*============================================================================*
 * Definição das variáveis globais (públicas ou privadas)                     *
 *============================================================================*/

/**
 * ATENÇÃO: Você pode adicionar novas funções com PUBLIC para serem usadas por
 * outros arquivos e adicionar a assinatura da função no .h referente.
 */

/**
 * @brief Definição de variáveis globais.
 */
PUBLIC partida_t * partida;
PUBLIC int tempo_partida;

/*============================================================================*
 * partida_setup()                                                            *
 *============================================================================*/

/**
 * @brief Configura a estrutura de partida.
 */
PUBLIC void partida_setup(void)
{
	assert((partida = (partida_t *) malloc(sizeof(partida_t))) != NULL);

	/**
	 * Setup Equipes.
	 */
	equipe_setup(&partida->equipe_a, EQUIPE_A, params->jogadores_por_equipe);
	equipe_setup(&partida->equipe_b, EQUIPE_B, params->jogadores_por_equipe);

	/**
	 * Setup Atributos da partida.
	 */
	partida->status = PARTIDA_NAO_PREPARADA;

	// settar tempo_partida
	tempo_partida = 0;

	/**
	 * Complemente se precisar.
	 */
}

/*============================================================================*
 * partida_cleanup()                                                          *
 *============================================================================*/

/**
 * @brief Limpa os recursos de uma partida.
 */
PUBLIC void partida_cleanup(void)
{
	/**
	 * Cleanup Equipes.
	 */
	equipe_cleanup(&partida->equipe_a);
	equipe_cleanup(&partida->equipe_b);

	/**
	 * Complemente se precisar.
	 */

	/* Libera partida. */
	free(partida);
}

/*============================================================================*
 * partida_status()                                                           *
 *============================================================================*/

/**
 * @brief Verifica o status da partida.
 *
 * @return Retorna o status da partida.
 */
PUBLIC partida_status_t partida_status(void)
{
	partida_status_t status;

	status = partida->status;

	return (status);
}

/*============================================================================*
 * partida_nomeia_vencedores()                                                *
 *============================================================================*/

/**
 * @brief Imprime equipe vencedora.
 */
PRIVATE void partida_print_resultado(int tempo_restante, int sobreviventes, partida_resultado_t res)
{
	switch (res)
	{
		case PARTIDA_RESULTADO_EQUIPE_A_VENCEU:
			plog("[resultado] Partida %d: Equipe A venceu com %d sobreviventes! (tempo %d/%d)\n",
				sim->partidas_jogadas,
				sobreviventes,
				tempo_restante,
				params->partida_tempo_max
			);
			break;

		case PARTIDA_RESULTADO_EQUIPE_B_VENCEU:
			plog("[resultado] Partida %d: Equipe B venceu com %d sobreviventes! (tempo %d/%d)\n",
				sim->partidas_jogadas,
				sobreviventes,
				tempo_restante,
				params->partida_tempo_max
			);
			break;

		case PARTIDA_RESULTADO_EMPATOU:
			plog("[resultado] Partida %d: Ocorreu um empate, quantidade de jogadores em cada equipe %d (tempo %d/%d)\n",
				sim->partidas_jogadas,
				sobreviventes,
				tempo_restante,
				params->partida_tempo_max
			);
			break;

		default:
			assert(false);
			break;
	}

	fflush(stdout);
}

/*============================================================================*
 * partida_nomeia_vencedores()                                                *
 *============================================================================*/

/**
 * @brief Verifica qual equipe venceu e imprime o resultado informando o tempo
 * restante da partida, a quantidade de sobreviventes na equipe vencedora (ou
 * em cada equipe se houver empate) e o número da partida.
 *
 * Condições de vitória:
 * - Se ambas as equipes tiverem a mesma quantidade de jogadores vivos, ocorre
 *   um empate.
 * - Se uma equipe tiver 0 jogadores vivos, a outra equipe vence.
 *
 * Utilize a função partida_print_resultado para te auxiliar.
 */
PUBLIC void partida_nomeia_vencedores(int tempo_restante)
{
	int vivosA = quantidade_vivos(partida->equipe_a);
	int vivosB = quantidade_vivos(partida->equipe_b);
	
	if (vivosA == vivosB) {
		// empate
		partida_print_resultado(tempo_restante, vivosA, PARTIDA_RESULTADO_EMPATOU);
	} else if (vivosA == 0) {
		// equipe B ganha
		partida_print_resultado(tempo_restante, vivosB, PARTIDA_RESULTADO_EQUIPE_B_VENCEU);
	} else if (vivosB == 0) {
		// equipe A ganha
		partida_print_resultado(tempo_restante, vivosA, PARTIDA_RESULTADO_EQUIPE_A_VENCEU);
	} else {
		// resultado indefinido
		partida_print_resultado(0, 0, PARTIDA_RESULTADO_INDEFINIDO);
	}
}

PUBLIC int quantidade_vivos(equipe_t equipe) {
	arranjo_t *jogadores = &equipe.jogadores;

	int vivos = 0;

	// pegar mortos da equipe
	for (int i = 0; i < arranjo_tamanho(jogadores); i++) {
		jogador_t *jogador = (jogador_t *) arranjo_at(jogadores, i);
		if (jogador->status == JOGADOR_JOGANDO) vivos++;
	}

	return vivos;
}

PUBLIC int are_todos_esperando(equipe_t equipe) {
	arranjo_t *jogadores = &equipe.jogadores;

	int esperando = 0;

	for (int i = 0; i < arranjo_tamanho(jogadores); i++) {
		jogador_t *jogador = (jogador_t *) arranjo_at(jogadores, i);
		if (jogador->status == JOGADOR_ESPERANDO) esperando++;
	}

	return arranjo_tamanho(jogadores) == esperando ? 1 : 0;
}