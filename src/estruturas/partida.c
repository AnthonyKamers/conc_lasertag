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
	partida->partida_now = 1;
	partida->tempo_partida = 0;
	partida->jogadores_equipes = 0;
	partida->jogadores_esperando = 0;

	// semáforos jogadores/partida
	sem_init(&partida->semaforo_wait_partida, 0, 2 * params->jogadores_por_equipe);
	sem_init(&partida->semaforo_equipamentos_disponiveis, 0, 2 * params->jogadores_por_equipe);
	sem_init(&partida->semaforo_comecar_partida, 0, 0);
	sem_init(&partida->semaforo_saindo_partida, 0, 0);

	// semáforos gerente (binário [quando avançar para próxima etapa])
	sem_init(&partida->semaforo_gerente_espera_equipes, 0, 0);
	sem_init(&partida->semaforo_gerente_jogadores_esperando, 0, 0);
	sem_init(&partida->semaforo_gerente_comeca_partida, 0, 0);

	// mutex do limpador
	pthread_mutex_init(&partida->mutex_limpador, NULL);

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
	free(partida);

	// semáforos jogadores/partida
	sem_destroy(&partida->semaforo_wait_partida);
	sem_destroy(&partida->semaforo_saindo_partida);
	sem_destroy(&partida->semaforo_comecar_partida);

	// semáforos gerente
	sem_destroy(&partida->semaforo_gerente_espera_equipes);
	sem_destroy(&partida->semaforo_gerente_jogadores_esperando);
	sem_destroy(&partida->semaforo_gerente_comeca_partida);

	// mutex limpador
	pthread_mutex_destroy(&partida->mutex_limpador);

	/**
	 * Complemente se precisar.
	 */

	/* Libera partida. */
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

int soma_vida_equipe(equipe_t equipe) {
	int somaVida = 0;

	arranjo_t *jogadores = (arranjo_t *) &equipe.jogadores;

	for (int i = 0; i < arranjo_tamanho(jogadores); i++) {
		jogador_t *jogador_now = (jogador_t *) jogadores->conteudo[i];

		if (jogador_now->status == JOGADOR_JOGANDO) somaVida++;
	}

	return somaVida;
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

	if (tempo_restante <= 0) {
		// tempo estourou
		if (vivosA > vivosB) {
			// equipe A tem mais vivos do que equipe B -> equipe A ganhou
			partida_print_resultado(tempo_restante, vivosA, PARTIDA_RESULTADO_EQUIPE_A_VENCEU);

		} else if (vivosB > vivosA) {
			// equipe B tem mais vivos do que equipe A -> equipe B ganhou
			partida_print_resultado(tempo_restante, vivosB, PARTIDA_RESULTADO_EQUIPE_B_VENCEU);

		} else if (vivosA == vivosB) {
			// se os vivos de cada equipe forem iguais -> ver qual equipe a soma da vida maior
			int somaVidaA = soma_vida_equipe(partida->equipe_a);
			int somaVidaB = soma_vida_equipe(partida->equipe_b);

			if (somaVidaA > somaVidaB) {
				// se a soma da vida da equipe A é maior do que a da equipe B -> equipe A ganhou
				partida_print_resultado(tempo_restante, vivosA, PARTIDA_RESULTADO_EQUIPE_A_VENCEU);

			} else if (somaVidaB > somaVidaA) {
				// se a soma da vida da equipe B é maior do que a da equipe A -> equipe B ganhou
				partida_print_resultado(tempo_restante, vivosB, PARTIDA_RESULTADO_EQUIPE_B_VENCEU);

			}
		} else {
			// resultado indefinido
			partida_print_resultado(0, 0, PARTIDA_RESULTADO_INDEFINIDO);
		}
	} else {
		// se tempo não estourou e acabou -> alguma equipe venceu
		if (vivosA == 0) {
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
}

PUBLIC arranjo_t *filtrar_jogadores(arranjo_t *jogadores, jogador_status_t status) {
	arranjo_t *jogadores_filtrados = malloc(sizeof(arranjo_t));
	arranjo_iniciar(jogadores_filtrados, params->jogadores_por_equipe, 0);

	for (int i = 0; i < arranjo_tamanho(jogadores); i++) {
		jogador_t *jogador_now = (jogador_t *) jogadores->conteudo[i];
		if (jogador_now->status == status) arranjo_colocar(jogadores_filtrados, jogador_now);
	}

	return jogadores_filtrados;
}

PUBLIC int quantidade_vivos_geral() {
	equipe_t equipeA = partida->equipe_a;
	equipe_t equipeB = partida->equipe_b;

	arranjo_t *jogadoresA = &equipeA.jogadores;
	arranjo_t *jogadoresB = &equipeB.jogadores;

	int vivosA = 0;
	int vivosB = 0;

	// como cada equipe tem mesmo número de jogadores,
	// fazer apenas um for	
	for (int i = 0; i < arranjo_tamanho(jogadoresA); i++) {
		jogador_t *jogadorA = (jogador_t *) jogadoresA->conteudo[i];
		jogador_t *jogadorB = (jogador_t *) jogadoresB->conteudo[i];
		if (jogadorA->status == JOGADOR_JOGANDO) vivosA++;
		if (jogadorB->status == JOGADOR_JOGANDO) vivosB++;
	}

	return vivosA + vivosB;
}

PUBLIC int quantidade_vivos(equipe_t equipe) {
	arranjo_t *jogadores = &equipe.jogadores;

	int vivos = 0;

	// pegar mortos da equipe
	for (int i = 0; i < arranjo_tamanho(jogadores); i++) {
		jogador_t *jogador = (jogador_t *) jogadores->conteudo[i];
		if (jogador->status == JOGADOR_JOGANDO) vivos++;
	}

	return vivos;
}

PUBLIC int are_todos_esperando() {
	arranjo_t *jogadoresA = (arranjo_t *) &partida->equipe_a.jogadores;
	arranjo_t *jogadoresB = (arranjo_t *) &partida->equipe_b.jogadores;

	int esperandoA = 0;
	int esperandoB = 0;

	for (int i = 0; i < arranjo_tamanho(jogadoresA); i++) {
		jogador_t *jogador = (jogador_t *) jogadoresA->conteudo[i];
		if (jogador->status == JOGADOR_ESPERANDO) esperandoA++;
	}

	for (int i = 0; i < arranjo_tamanho(jogadoresB); i++) {
		jogador_t *jogador = (jogador_t *) jogadoresB->conteudo[i];
		if (jogador->status == JOGADOR_ESPERANDO) esperandoB++;
	}

	return (esperandoA + esperandoB) == 2 * params->jogadores_por_equipe ? 1 : 0;
}