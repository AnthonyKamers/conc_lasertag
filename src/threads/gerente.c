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

PUBLIC int gerente_partida_acabou(void);
PUBLIC void gerente_reset_partida(void);

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
	while (partida->partida_now <= params->partidas_max) {
		// esperar todos os jogadores entrarem em equipes
		sem_wait(&partida->semaforo_gerente_espera_equipes);

			plog("gerente liberou para PARTIDA_PREPARADA \n");
			partida->status = PARTIDA_PREPARADA;

			// esperar todos os jogadores estarem esperando
			// setta básico da partida (tempo e status)
			// chama while de gerente_coordena_partida() para fazer handle de fim de jogo e 
			// cura de jogadores no meio da partida
			sem_wait(&partida->semaforo_gerente_jogadores_esperando);
				// dar (jogadores_por_equipe * 2) posts em partida->semaforo_wait_partida
				// para permitir jogadores entrarem na partida e jogar
				plog("gerente vai fazer 2 * params->jogadores_por_equipe posts em semaforo_wait_partida \n");
				for (int i = 0; i < 2 * params->jogadores_por_equipe; i++) {
					sem_post(&partida->semaforo_comecar_partida);
				}

				// setta inicio da partida
				sim->partidas_jogadas += 1;  // diz para o simulador que mais uma partida foi jogada
				partida->tempo_partida = 0;
				partida->status = PARTIDA_INICIADA;

				// chama while para curar jogadores e testar fim da partida
				gerente_coordena_partida();

				// tenta ver se pode começar nova partida
				sem_wait(&partida->semaforo_gerente_comeca_partida);

				// resettar todas as configurações da partida
				gerente_reset_partida();
	}

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

	plog("[gerente] Curando jogadores.\n");

	arranjo_t *jogadoresA = &partida->equipe_a.jogadores;
	arranjo_t *jogadoresB = &partida->equipe_b.jogadores;

	for (int i = 0; i < params->jogadores_por_equipe; i++) {
		jogador_t *jogadorNowA = (jogador_t *) jogadoresA->conteudo[i];
		jogador_t *jogadorNowB = (jogador_t *) jogadoresB->conteudo[i];

		// curar jogadorA[i] e jogadorB[i]
		jogadorNowA->vida =
			jogadorNowA->vida + params->dano_cura > 100 ?
			100 : 
			jogadorNowA->vida + params->dano_cura;

		curados++;

		jogadorNowB->vida =
			jogadorNowB->vida + params->dano_cura > 100 ?
			100 : 
			jogadorNowB->vida + params->dano_cura;
		curados++;
	}

	return (curados);
}

/*============================================================================*
 * gerente_partida_acabou()                                                 *
 *============================================================================*/

PUBLIC int gerente_partida_acabou(void) {
	// testa se acabou a partida
	equipe_t equipeA = partida->equipe_a;
	equipe_t equipeB = partida->equipe_b;

	if (
		partida->tempo_partida >= params->partida_tempo_max ||
		quantidade_vivos(equipeA) <= 0 ||
		quantidade_vivos(equipeB) <= 0
	) {
		// plog("gerente vai terminar partida | tempo_partida = %d | vivosA = %d | vivosB = %d \n",
		// 	partida->tempo_partida,
		// 	quantidade_vivos(equipeA),
		// 	quantidade_vivos(equipeB)
		// );
		
		// acaba com a partida (jogadores vão parar de jogar aqui)
		partida->status = PARTIDA_FINALIZADA;

		int tempo_restante = params->partida_tempo_max - partida->tempo_partida;
		partida_nomeia_vencedores(tempo_restante);

		// dar 2 * jogadores_por_equipe post em partida->semaforo_saindo_partida
		// para liberar jogadores para conseguir sair da partida
		for (int i = 0; i < 2 * params->jogadores_por_equipe; i++) {
			sem_post(&partida->semaforo_saindo_partida);
		}

		// libera semáforo para gerente poder continuar e
		// criar nova partida
		sem_post(&partida->semaforo_gerente_comeca_partida);

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
	// e houver jogadores de ambas equipes vivos:
	// coordena a partida
	int rodando = 1;
	while (rodando) {
		msleep(params->delay_gerente);
		partida->tempo_partida += params->delay_gerente;
		sim->tempo_jogado += params->delay_gerente;

		int curados = gerente_cura_jogadores();
		sim->jogadores_curados += curados;  // adiciona quantidade de curados à simulação

		rodando = gerente_partida_acabou();
	}
}


/*============================================================================*
 * gerente_reset_partida()                                                 *
 *============================================================================*/

PUBLIC void gerente_reset_partida(void) {
	// resetta todos os parâmetros da partida
	partida->status = PARTIDA_NAO_PREPARADA;
	partida->tempo_partida = 0;
	partida->jogadores_esperando = 0;
	partida->jogadores_equipes = 0;
	partida->partida_now += 1;

	// abre espaços para novos jogadores conseguirem entrar na partida
	for (int i = 0; i < 2 * params->jogadores_por_equipe; i++) {
		sem_post(&partida->semaforo_wait_partida);
	}
}
