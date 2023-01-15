# Simulador de Aviação Controlado por um Acelerómetro

Projeto desenvolvido no âmbito da unidade curricular de instrumentação avançada do mestrado em engenharia física da Universidade do Minho.

<p align="center">
  <img src="media/demo.gif" alt="animated" width=70% center/>
</p>

## Introdução

Nos dias de hoje, a utilização de sistemas ciberfísicos e o interesse nestes por parte das sociedades tem aumentado. Na sua essência, estes sistemas são projetados para interagirem com o mundo físico e realizar tarefas específicas, como automação de sistemas, o que implica o controlo e monitorização de variáveis físicas em constante alteração. As variáveis físicas, usualmente de natureza analógica, são detetadas através de sensores e traduzidas, por estes, para sinais elétricos. Estes sinais serão posteriormente processados conforme a aplicação desejada. Normalmente, recorrem-se a microcontroladores para receberem a informação dos sensores e, posteriormente, enviarem para unidades de processamento com maior capacidade de processamento. É importante não esquecer que estes sistemas também englobam atuadores, ou seja, o processo inverso, em que é a unidade de processamento que, por algum motivo, envia indicações ao microcontrolador para ativar algum atuador.


Neste projeto, no âmbito da unidade curricular de instrumentação avançada do mestrado em engenharia física da Universidade do Minho, desenvolveu-se um simulador de aviação controlado por um acelerómetro. Para tal foi necessário programar um microcontrolador do modelo PIC18F47Q10. A aplicação projetada controlará o movimento de um avião em ambiente de simulação, sendo assim necessária a obtenção de valores para a direção do movimento do objeto. Por simplicidade de utilização do simulador, definiu-se que o avião movimentar-se-á a uma velocidade constante e que a direção do movimento deste deverá ser paralela à do acelerómetro. Com isto, utilizar-se-á a informação proveniente dos 3 eixos do acelerómetro para determinar a direção de movimento do avião. O protocolo de comunicação utilizado entre a STIM (microcontrolador) e a NCAP (unidade com maior capacidade de processamento), tal como será posteriormente explicado, foi o IEEE 1451.0.

## Notas adicionais

O relatório final pode ser encontrado [aqui](final_report.pdf).

## Autores

* [Gabriela Jordão](mgabijo@gmail.com)
* [Miguel Peixoto](miguelpeixoto457@gmail.com)


## Licença

Este projeto está licenciado sob a licença MIT - consulte o ficheiro [LICENSE](LICENSE) para mais detalhes.