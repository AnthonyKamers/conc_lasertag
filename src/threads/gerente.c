#include "config.h"
#include "partida.h"
#include "jogador.h"

/**
 * ATENÇÃO: Você pode adicionar novas funções com PUBLIC para serem usadas por
 * outros arquivos e adicionar a assinatura da função no .h referente.
 */

/*============================================================================*
 * Protótipo das funções locais.                                              *
 *============================================================================*/

PRIVATE void gerente_coordena_partida(void);

/*============================================================================*
 * gerente_setup()                                                            *
 *============================================================================*/

/**
 * @brief Configura os recursos locais do gerente se houverem.
 */
PUBLIC void gerente_setup(void)
{
}

/*============================================================================*
 * gerente_cleanup()                                                          *
 *============================================================================*/

/**
 * @brief Limpa os recursos locais do gerente se houverem.
 */
PUBLIC void gerente_cleanup(void)
{
}

/*============================================================================*
 * Responsabilidades do Gerente.                                              *
 *============================================================================*/

/**
 * @brief Main do gerente.
 *
 * Descrição: O Gerente deve coordenar as partidas, ou seja:
 * - Esperar que as equipes se formarem.
 * - Iniciar a partida.
 * - Enquanto o tempo de partida não ultrapassar params->partida_tempo_max
 *   e ainda existem jogadores vivos em ambas as equipes:
 * --- Esperar params->delay_gerente
 * --- Perioricamente, curar todos os jogadores vivos com params->dano_cura.
 *
 * ATENÇÃO: Você deve incrementar os contadores do sim_t referente a partida
 * e jogadores curados.
 */
PUBLIC void * gerente_fn(void * arg)
{
	plog("[gerente] Iniciou.\n");

	// enquanto a quantidade máxima de partidas não foi atingida
	while (partida->partida_now < params->partidas_max) {
		// esperar todos os jogadores entrarem em equipes
		sem_wait(&partida->semaforo_gerente_espera_equipes);
			partida->status = PARTIDA_PREPARADA;

			// esperar todos os jogadores estarem esperando
			// setta básico da partida (tempo e status)
			// chama while de gerente_coordena_partida() para fazer handle de fim de jogo e 
			// cura de jogadores no meio da partida
			sem_wait(&partida->semaforo_gerente_jogadores_esperando);
				// dar (jogadores_por_equipe * 2) posts em partida->semaforo_wait_partida
				for (int i = 0; i < 2 * params->jogadores_por_equipe; i++) {
					sem_post(&partida->semaforo_wait_partida);
				}

				// setta inicio da partida
				partida->tempo_partida = 0;
				partida->status = PARTIDA_INICIADA;

				// chama while para curar jogadores e testar fim da partida
				gerente_coordena_partida();
	}

	// sem_wait(&partida->semaforo_wait_partida);  // quando todos estiverem jogando, vai conseguir dar wait
	// 	partida->tempo_partida = 0;
	// 	partida->status = PARTIDA_INICIADA;
	// 	while (1) {
	// 		msleep(params->delay_gerente);
	// 		partida->tempo_partida += params->delay_gerente;

	// 		gerente_coordena_partida();

	// 		// testa se acabou a partida
	// 		equipe_t equipeA = partida->equipe_a;
	// 		equipe_t equipeB = partida->equipe_b;

	// 		if (
	// 			partida->tempo_partida < params->partida_tempo_max &&
	// 			quantidade_vivos(equipeA) > 0 &&
	// 			quantidade_vivos(equipeB) > 0
	// 		) {
	// 			// acaba com a partida
	// 			partida->status = PARTIDA_FINALIZADA;
	// 			partida_nomeia_vencedores(partida->tempo_partida);
	// 			partida->tempo_partida = 0;

	// 			break;
	// 		}
	// 	}

	// // enquanto o tempo da partida não ultrapassar o tempo máximo (para cada partida)
	// while (tempo_partida <= params->partida_tempo_max) {
	// 	// tick
	// 	msleep(params->delay_gerente);
	// 	tempo_partida += params->delay_gerente;

	// 	equipe_t equipeA = partida->equipe_a;
	// 	equipe_t equipeB = partida->equipe_b;

	// 	// se as equipes estiverem prontas -> inicia a partida
	// 	if (
	// 		(arranjo_tamanho(&equipeA.jogadores) == equipeA.capacidade) &&
	// 		(arranjo_tamanho(&equipeB.jogadores) == equipeB.capacidade) &&
	// 		partida->status == PARTIDA_NAO_PREPARADA
	// 	) {
	// 		partida->status = PARTIDA_PREPARADA;
	// 	}

	// 	// settar de PREPARADA para INICIADA [se todos jogadores estiverem esperando]
	// 	// printf("are_todos_esperando() from gerente = %d \n", are_todos_esperando());
	// 	if (
	// 		are_todos_esperando() &&
	// 		partida->status == PARTIDA_PREPARADA
	// 	) {
	// 		plog("gerente vai iniciar partida \n");
	// 		partida->status = PARTIDA_INICIADA;
	// 	}

	// 	// se uma equipe está toda morta ou se o tempo acabou
	// 	//	-> setta partida para FINALIZADA
	// 	//	-> chama partida_nomeia_vencedores()
	// 	if (
	// 		partida->status == PARTIDA_INICIADA &&
	// 		(
	// 			(quantidade_vivos(equipeA) == 0 && arranjo_tamanho(&equipeA.jogadores) > 0) ||
	// 			(quantidade_vivos(equipeB) == 0 && arranjo_tamanho(&equipeB.jogadores) > 0) ||
	// 			tempo_partida >= params->partida_tempo_max
	// 		)
	// 	) {
	// 		partida->status = PARTIDA_FINALIZADA;
	// 		partida_nomeia_vencedores(tempo_partida);
	// 	}

	// 	// chama coordena_partida (para curar jogadores)
	// 	gerente_coordena_partida();
	// }

	if (false)
		gerente_coordena_partida();

	return (NULL);
}

/*============================================================================*
 * gerente_cura_jogadores()                                                   *
 *============================================================================*/

/**
 * @brief Gerente cura os jogadores.
 */
PRIVATE int gerente_cura_jogadores(void)
{
	int curados = 0;
	int qtdGeral = params->jogadores_por_equipe;

	plog("[gerente] Curando jogadores.\n");

	arranjo_t *jogadoresA = &partida->equipe_a.jogadores;
	arranjo_t *jogadoresB = &partida->equipe_b.jogadores;

	for (int i = 0; i < qtdGeral; i++) {
		jogador_t *jogadorNowA = (jogador_t *) jogadoresA->conteudo[i];
		jogador_t *jogadorNowB = (jogador_t *) jogadoresB->conteudo[i];

		// curar jogadorA[i] e jogadorB[i]
		jogadorNowA->vida += params->dano_cura;
		sim->jogadores_curados++;

		jogadorNowB->vida += params->dano_cura;
		sim->jogadores_curados++;
	}

	return (curados);
}

/*============================================================================*
 * gerente_partida_acabou()                                                 *
 *============================================================================*/

PUBLIC int gerente_partida_acabou() {
	// testa se acabou a partida
	equipe_t equipeA = partida->equipe_a;
	equipe_t equipeB = partida->equipe_b;

	if (
		partida->tempo_partida >= params->partida_tempo_max ||
		quantidade_vivos(equipeA) > 0 ||
		quantidade_vivos(equipeB) > 0
	) {
		// acaba com a partida
		partida->status = PARTIDA_FINALIZADA;
		partida_nomeia_vencedores(partida->tempo_partida);

		// dar 2 * jogadores_por_equipe post em partida->semaforo_saindo_partida
		for (int i = 0; i < 2 * params->jogadores_por_equipe; i++) {
			sem_post(&partida->semaforo_saindo_partida);
		}

		// resetta a partida
		// partida->partida_now += 1;
		// partida->tempo_partida = 0;
		// partida->jogadores_esperando = 0;
		// partida->status = PARTIDA_NAO_PREPARADA;

		return 0;
	}

	return 1;
}

/*============================================================================*
 * gerente_coordena_partida()                                                 *
 *============================================================================*/

/**
 * @brief Loop principal da partida.
 */
PRIVATE void gerente_coordena_partida(void)
{
	plog("[gerente] Coordenando partida.\n");

	// enquanto houver tempo de partida
	// coordena a partida
	int rodando = 1;
	while (rodando) {
		msleep(params->delay_gerente);
		partida->tempo_partida += params->delay_gerente;

		gerente_cura_jogadores();

		rodando = gerente_partida_acabou();
	}

	// equipe_t equipeA = partida->equipe_a;
	// equipe_t equipeB = partida->equipe_b;

	// // enquanto houver tempo de partida e partida estiver rodando
	// //	-> cura periodicamente os jogadores
	// if (
	// 	partida->tempo_partida < params->partida_tempo_max &&
	// 	quantidade_vivos(equipeA) > 0 &&
	// 	quantidade_vivos(equipeB) > 0
	// ) {
	// 	gerente_cura_jogadores();
	// }
	// if (false)
	// 	gerente_cura_jogadores();
}

