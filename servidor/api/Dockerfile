# Imagem Docker da API PlantCare
# Construída pelo docker-compose.yml (serviço "api"): docker compose up -d --build

# Ponto de partida: Debian mínimo com Python 3.13
FROM python:3.13-slim

# Não criar ficheiros .pyc e mostrar os logs logo, sem ficarem em espera
ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1

# Pasta de trabalho dentro do contentor
WORKDIR /app

# 1.º só a lista de bibliotecas, e instalá-las.
#    O Docker guarda cada passo em cache: enquanto o requirements.txt não mudar,
#    este passo (o mais demorado) não se repete quando alteras o código.
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# 2.º o código da API
COPY . .

# Correr com um utilizador sem privilégios, e não como root:
# se alguém descobrir uma falha na API, fica com menos poder dentro do contentor.
RUN useradd --create-home plantcare
USER plantcare

EXPOSE 8000

# --host 0.0.0.0: aceitar ligações vindas de FORA do contentor
# (com 127.0.0.1, só o próprio contentor conseguia falar com a API)
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]