#include "bsp_usart.h"
#include <stdio.h>
#include "bsp_dma.h"


uint8_t recive_buff[USART_RECIVE_LENGTH];
uint16_t recive_length =0;
uint8_t recive_complete =0;//0δ��ɣ�1���




void usart_config(uint32_t baudrate)
{
	rcu_periph_clock_enable(RCU_USART0);//ʱ��ʹ��
	rcu_periph_clock_enable(BSP_USART_RX_RCU);//GPIOAʱ��ʹ��
	rcu_periph_clock_enable(BSP_USART_TX_RCU);
	
	gpio_af_set(BSP_USART_TX_PORT,BSP_USART_AF,BSP_USART_TX_PIN);//���ù�������(TXD--PA9)
	gpio_af_set(BSP_USART_RX_PORT,BSP_USART_AF,BSP_USART_RX_PIN);
	
	gpio_mode_set(BSP_USART_TX_PORT,GPIO_MODE_AF,GPIO_PUPD_PULLUP,BSP_USART_TX_PIN);//uart���иߵ�ƽ
	gpio_mode_set(BSP_USART_RX_PORT,GPIO_MODE_AF,GPIO_PUPD_PULLUP,BSP_USART_RX_PIN);

	gpio_output_options_set(BSP_USART_TX_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,BSP_USART_TX_PIN);//PP����PD��©
	gpio_output_options_set(BSP_USART_RX_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,BSP_USART_RX_PIN);
	
	usart_deinit(USART0);//��ʼ��
	usart_baudrate_set(USART0,baudrate);//������
	usart_parity_config(USART0,USART_PM_NONE);//У��λ
	usart_word_length_set(USART0,USART_WL_8BIT);//����λ
	usart_stop_bit_set(USART0,USART_STB_1BIT);//ֹͣλ
	
	usart_enable(USART0);
	usart_transmit_config(USART0,USART_TRANSMIT_ENABLE);
	usart_receive_config(USART0,USART_RECEIVE_ENABLE);//����ʹ��
	
	nvic_irq_enable(USART0_IRQn,2,2);
	#if !USE_USART_DMA
	usart_interrupt_enable(USART0,USART_INT_RBNE);
	#endif
	usart_interrupt_enable(USART0,USART_INT_IDLE);
	

	
}

void usart_send (uint32_t data)
{
	usart_data_transmit(USART0,(uint32_t)data);
	while(RESET==usart_flag_get(USART0,USART_FLAG_TBE))//��Ϊ�����ڴ�ѭ��
	{}
}

void usart_send_string(uint8_t *string)
{
	while(string && *string)
	{
		usart_send(*string++);
	}
}

int fputc (int ch,FILE *f)
{
	usart_send(ch);
	return ch;
}

void USART0_IRQHandler(void)
{
	#if !USE_USART_DMA
	if(SET == usart_interrupt_flag_get(USART0,USART_INT_FLAG_RBNE))
	{
		recive_buff[recive_length++]=usart_data_receive(USART0);//���ڽ����Դ������־λ
	}
	#endif
	if(SET == usart_interrupt_flag_get(USART0,USART_INT_FLAG_IDLE))
	{
		usart_data_receive(USART0);//���ô��ڽ��պ��������־λ
		#if USE_USART_DMA
		recive_length=USART_RECIVE_LENGTH-dma_transfer_number_get(DMA1,DMA_CH2);
		
		dma_channel_disable(DMA1,DMA_CH2);
		dma1_config();
		#endif
		recive_buff[recive_length]='\0';
		recive_complete = 1;
		
	}
}

