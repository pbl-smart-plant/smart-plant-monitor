# PlantCare — Proposta Inicial (Milestone 1)

## 1. Identificação

* **Universidade:** Universidade Europeia
* **Faculdade:** IADE - Faculdade de Design, Tecnologia e Comunicação
* **Curso:** Engenharia Informática - EaD 2026/2027
* **Unidades Curriculares:** 
  * Sistemas Distribuídos
  * Computação Física e IoT
  * Engenharia de Software
  * Inteligência Artificial

* **Nome do Projeto:** PlantCare - Monitor de bem estar para plantas
* **Grupo:** 
* **Elementos do Grupo:**
  * Daniel Cruz (20240455) 
  * Rui Encarnação (20240458) 
  * Dylane Esteves(20240914)  
  * Rute Frutuoso (20241693)

* **Link para o Repositório GitHub:** https://github.com/pbl-smart-plant/smart-plant-monitor
* **Palavras-chave:** IoT, ESP32, Inteligência Artificial, Sistemas Distribuídos, Casas Inteligentes, Plantas.

---

## 2. Descrição e Contextualização

### 2.1. Descrição do Projeto e Problema a Resolver
Ter um pequeno jardim em casa pode ajudar a equilibrar o bem-estar que, cada vez mais, pessoas procuram no seu lar. No entanto, o ritmo acelerado do dia-a-dia deixa qualquer um com os horários trocados e não é difícil perder a noção de: "foi neste fim de semana que reguei as plantas?" quando na realidade já foi há três semanas.
Pretendemos, com este projeto, ajudar todos os PlantLovers a estarem mais atentos às necessidades das suas plantas, mesmo que o seu dia-a-dia seja num ritmo mais acelerado do que aquilo que eles gostariam.

### 2.2. Motivação e Objetivos
* **Motivação:** Tendo vários PlantLovers na constituição do grupo, foi fácil chegar à ideia deste projeto, pois tem sido um dos desafios ao longo do curso: manter todas as plantas de casa vivas.
* **Objetivos:** 
  * [Objetivo 1: Criar protótipo IoT baseado em ESP32 para recolha de dados.]
  * [Objetivo 2: Implementar servidor web e base de dados relacional.]
  * [Objetivo 3: Desenvolver modelo preditivo de IA para sugerir um melhor cuidado de determinada planta.]

### 2.3. Público-Alvo
Amantes de plantas, com vidas activas e agitadas.

### 2.4. Enquadramento nas Unidades Curriculares
* **Sistemas Distribuídos:** Gestão de projeto, infraestrutura de comunicação e integração dos serviços.
* **Computação Física e IoT:** Programação do ESP32, leitura de sensores e envio de dados via REST.
* **Inteligência Artificial:** Tratamento de dados de sensores e fontes externas para treino de modelos de preditivos de cuidados.
* **Engenharia de Software:** Especificação de requisitos funcionais e não funcionais e definição dos guiões de teste.

---

## 3. Requisitos e Especificações Técnicas

### 3.1. Requisitos Técnicos e Tecnologias (Provisórios)
* **Hardware:** Microcontrolador ESP32, sensores de corrente/tensão, atuadores (relés), breadboard, cabos.
* **Software & Linguagens:** C/C++ (Arduino IDE / PlatformIO), Python (Treino de IA), SQL (MySQL), Node.js / Express (Servidor Web REST).
* **Plataformas & Ferramentas:** GitHub, Wokwi, Diagrams.net, EasyEDA, VS Code.

### 3.2. Lista Preliminar de Material
| Componente | Quantidade | Descrição / Função |
| ESP32 | 1 | Microcontrolador principal com Wi-Fi |
| Painel Solar | 2 | Fonte de energia auxiliar |
| Powerbank | 1 | Fonte de alimentação do Arduino |
| Sensor de Humidade | 1 | Controlo de humidade do ar |
| Sensor de Humidade do Solo | 1 | Controlo de humidade do solo |
| Sensor de Luz | 1 | Controlo de luminosidade |
| Sensor de pH | 1 | Controlo do pH do solo |
| Sensor de Temperatura | 1 | Controlo de temperatura do ar |

### 3.3. Guiões de Teste Preliminares
* **Guião de Teste 1: Registo e Envio de Dados do Sensor**
  1. O utilizador liga o sistema IoT à corrente elétrica e à rede Wi-Fi.
  2. O ESP32 lê os valores dos sensores.
  3. O ESP32 envia os dados formatados via post HTTP REST para o servidor.
  4. O servidor valida o post e armazena os dados na base de dados.
  5. *Resultado Esperado:* Os dados aparecem corretamente registados com timestamp na base de dados.

* **Guião de Teste 2: Obtenção de Sugestões de Cuidados Personalizados**
  1. O utilizador acede à interface do sistema ou consulta o endpoint de sugestões.
  2. O modelo de Inteligência Artificial processa o histórico de dados dos sensores e os dados adicionais externos.
  3. O sistema cria indicações dos cuidados a ter.
  4. *Resultado Esperado:* São apresentadas até 3 indicações dos cuidados mais imediatos a implementar.

---

## 4. Arquitetura e Design da Solução

### 4.1. Arquitetura Computacional (Provisória)
[Inserir ou anexar diagrama de blocos mostrando a comunicação: ESP32 -> API REST / Servidor Web -> Base de Dados MySQL -> Modelo IA / Dashboard]

### 4.2. Esboço do Artefacto Físico
[Inserir imagem/esboço do invólucro ou estrutura física que conterá o ESP32 e os sensores.]

### 4.3. Diagramas de Circuitos (Opcional nesta fase)
[Inserir esquema elétrico do circuito no Wokwi ou EasyEDA, caso já esteja elaborado.]

---

## 5. Planeamento e Calendarização

### 5.1. Distribuição de Tarefas
* **Sistemas Distribuídos & Infraestrutura:** [Nome do Elemento]
* **Computação Física & Hardware IoT:** [Nome do Elemento]
* **Desenvolvimento do Modelo de IA:** [Nome do Elemento]
* **Engenharia de Software & Testes:** [Nome do Elemento]

### 5.2. Plano de Trabalho / Gráfico de Gantt
