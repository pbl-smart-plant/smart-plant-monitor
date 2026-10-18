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
* **Grupo:** XX
* **Elementos do Grupo:**
  * Daniel Cruz (20240455) 
  * Rui Encarnação (20240458) 
  * Dylane Esteves(20240914)  
  * Rute Frutuoso (20241693)

* **Link para o Repositório GitHub:** [https://github.com/usuario/repositorio](https://github.com/usuario/repositorio)
* **Palavras-chave:** IoT, ESP32, Casas Inteligentes, Previsão Energética, Inteligência Artificial, Sistemas Distribuídos, Plantas.

---

## 2. Descrição e Contextualização

### 2.1. Descrição do Projeto e Problema a Resolver
Ter um pequeno jardim em casa pode ajudar a equilibrar o bem-estar que, cada vez mais, pessoas procuram no seu lar. No entanto, o ritmo acelerado do dia-a-dia deixa qualquer um com os horários trocados e não é difícil perder a noção de: "foi neste fim de semana que reguei as plantas?" quando na realidade já foi há três semanas.
Pretendemos, com este projeto, ajudar todos os PlantLovers a estarem mais atentos às necessidades das suas plantas, mesmo que o seu dia-a-dia seja num ritmo mais acelerado do que aquilo que eles gostariam.

### 2.2. Motivação e Objetivos
* **Motivação:** Tendo vários PlantLovers na constituição do grupo, foi fácil chegar à ideia deste projeto, pois tem sido um dos desafios ao longo do curso: manter todas as plantas de casa vivas.
* **Objetivos:** 
  * [Objetivo 1: Criar protótipo IoT baseado em ESP32 para recolha de dados.]
  * [Objetivo 2: Desenvolver modelo preditivo de IA para estimar o consumo de energia.]
  * [Objetivo 3: Implementar servidor web e base de dados relacional (MySQL).]

### 2.3. Público-Alvo
Amantes de plantas, com vidas activas e agitadas.

### 2.4. Enquadramento nas Unidades Curriculares
* **Sistemas Distribuídos:** Gestão de projeto, infraestrutura de comunicação e integração dos serviços.
* **Computação Física e IoT:** Programação do ESP32, leitura de sensores e envio de dados via REST.
* **Inteligência Artificial:** Tratamento de dados de sensores e fontes externas para treino de modelos preditivos de consumo.
* **Engenharia de Software:** Especificação de requisitos funcionais e não funcionais e definição dos guiões de teste.

---

## 3. Requisitos e Especificações Técnicas

### 3.1. Requisitos Técnicos e Tecnologias (Provisórios)
* **Hardware:** Microcontrolador ESP32, sensores de corrente/tensão, atuadores (relés), breadboard, cabos.
* **Software & Linguagens:** C/C++ (Arduino IDE / PlatformIO), Python (Treino de IA), SQL (MySQL), Node.js / Express (Servidor Web REST).
* **Plataformas & Ferramentas:** GitHub, Wokwi (Simulação), Diagrams.net, EasyEDA.

### 3.2. Lista Preliminar de Material
| Componente | Quantidade | Descrição / Função |
| ESP32 | 1 | Microcontrolador principal com Wi-Fi |
| Sensor de Corrente (ex: SCT-013) | 1 | Medição da corrente elétrica residencial |
| Módulo Relé | 1 | Atuador para controlo de cargas/eletrodomésticos |
| Fonte de Alimentação | 1 | Alimentação do circuito |

### 3.3. Guiões de Teste Preliminares (Pelo menos 2)
* **Guião de Teste 1: Registo e Envio de Dados do Sensor**
  1. O utilizador liga o sistema IoT à corrente elétrica e à rede Wi-Fi.
  2. O ESP32 lê os valores de corrente através do sensor.
  3. O ESP32 envia os dados formatados via pedido HTTP REST para o servidor.
  4. O servidor valida o pedido e armazena os dados na base de dados MySQL.
  5. *Resultado Esperado:* Os dados aparecem corretamente registados com timestamp na base de dados.

* **Guião de Teste 2: Obtenção de Previsão de Consumo**
  1. O utilizador acede à interface do sistema ou consulta o endpoint de previsão.
  2. O modelo de Inteligência Artificial processa o histórico de dados dos sensores e os dados adicionais externos.
  3. O sistema calcula a estimativa de consumo energético para as próximas horas.
  4. *Resultado Esperado:* É apresentada a estimativa energética ao utilizador de forma clara e legível.

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
