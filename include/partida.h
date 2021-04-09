#ifndef __PARTIDA_H__
#define __PARTIDA_H__

	#include "config.h"
	#include "equipe.h"
	#include "jogador.h"

	/*============================================================================*
	 * Estrutura                                                                  *
	 *============================================================================*/

	/**
	 * @brief Estados de uma partida.
	 */
	typedef enum PartidaStatus
	{
		PARTIDA_NAO_PREPARADA,
		PARTIDA_PREPARADA,
		PARTIDA_INICIADA,
		PARTIDA_FINALIZADA
	} partida_status_t;

	/**
	 * @brief Resultados de uma partida.
	 */
	typedef enum PartidaResultado
	{
		PARTIDA_RESULTADO_EQUIPE_A_VENCEU,
		PARTIDA_RESULTADO_EQUIPE_B_VENCEU,
		PARTIDA_RESULTADO_EMPATOU,
		PARTIDA_RESULTADO_INDEFINIDO
	} partida_resultado_t;

	/**
	 * @brief Estrutura da uma partida.
	 */
	typedef struct partida
	{
		/**
		 * @brief Equipes.
		 */
		equipe_t equipe_a;         /**< Equipe A.          */
		equipe_t equipe_b;         /**< Equipe B.          */

		/**
		 * @brief Estados de uma partida.
		 */
		partida_status_t status;   /**< Estado da partida. */

		// mutex para controlar qual jogador está higienizando os equipamentos
		// limpador pega um equipamento por vez
		pthread_mutex_t mutex_limpador;

		// semáforos de controle da partida (jogadores)
		sem_t semaforo_wait_partida;				// semáforo que vê que todos os jogadores estão esperando
		sem_t semaforo_saindo_partida;				// semáforo que conta quantos saíram da partida
		sem_t semaforo_equipamentos_disponiveis;	// semáforo que diz quantos equipamentos estão disponíveis na prateleira

		int partida_now;				// número da partida que está jogando agora
		int tempo_partida;				// tempo da partida que está jogando agora
		int jogadores_equipes;			// contador de jogadores que entraram para equipes
		int jogadores_esperando;		// contador de jogadores esperando

		// semáforos gerente
		sem_t semaforo_gerente_espera_equipes;			// semáforo para ver quando o gerente deve avançar
		sem_t semaforo_gerente_jogadores_esperando;		// semáforo para ver quando o gerente deve avançar
		sem_t semaforo_gerente_comeca_partida;			// semáforo para ver quando o gerente deve começar nova partida

		/**
		 * @brief Complemente se precisar.
		 */
	} partida_t;

	/**
	 * @brief Variável global da partida.
	 *
	 * @details Essa variável agrupa as equipes e variáveis de proteção para
	 * coordenar as ações dos jogadores.
	 */
	EXTERN partida_t * partida;

	/*============================================================================*
	 * Funções padrão exportadas (utilizadas em arquivos que incluem esse .h)     *
	 *============================================================================*/

	/**
	 * @brief Funções.
	 *
	 * @TODO: Verifique no arquivo src/estruturas/partida.c quais funções você
	 * deve implementar/complementar.
	 */
	EXTERN void partida_setup(void);
	EXTERN void partida_cleanup(void);
	EXTERN int partidas_restantes(void);
	EXTERN partida_status_t partida_status(void);
	EXTERN void partida_nomeia_vencedores(int);
	EXTERN int quantidade_vivos(equipe_t equipe);
	EXTERN int quantidade_vivos_geral();
	EXTERN int are_todos_esperando();
	EXTERN arranjo_t filtrar_jogadores(arranjo_t *arranjo, jogador_status_t status);

	/*============================================================================*
	 * Funções extra exportadas (utilizadas em arquivos que incluem esse .h)      *
	 *                                                                            *
	 * ATENCÃO: Insira aqui funções que você quiser adicionar a interface para    *
	 * serem usadas em arquivos que incluem esse header.                          *
	 *                                                                            *
	 * Você pode defini-las EXTERN assinatura (.h) + PUBLIC implementação (.c)    *
	 * ou usar diretamente extern assinatura (.h) + implementacao (.c).           *
	 * O uso dessas macros são para avaliação das funções já estabelecidas e não  *
	 * de novas funções que você precisar.                                        *
	 *============================================================================*/

#endif /*__PARTIDA_H__*/

