"""Formar os dados que a API recebe

Usa estas classes para validar cada pedido ANTES de chegar ao nosso código.
Se um campo faltar ou estiver fora dos limites, responde logo 400 com os detalhes.
Os limites são os mesmos das regras CHECK da BD (defesa em profundidade).
"""

from datetime import datetime
from typing import Optional

from pydantic import BaseModel, Field

class Leitura(BaseModel):
    """POST /api/Leituras"""
    data_hora: datetime  # hora da MEDIÇÃO, em UTC
    humidade_solo: float = Field(ge=0, le=100) # %  (ge = maior ou igual, le = menor ou igual)
    temperatura: float = Field(ge=-20, le=60) # °C
    humidade_ar: float = Field(ge=0, le=100) # %
    luz: int = Field(ge=0)
    nivel_agua: Optional[int] = Field(default=None, ge=0, le=4095) # só de hora a hora

class PedidoPin(BaseModel):
    """POST /api/dispositivos/pin"""
    pin: str = Field(pattern=r"^\d{6}$") # exatamente 6 algarismos

class PedidoRegisto(BaseModel):
    """POST /api/plantas/registar"""
    codigo: str = Field(min_length=1, max_length=20)
    pin: str = Field(pattern=r"^\d{6}$")
    email: str = Field(min_length=3, max_length=255) # vem do Grafana: ${__user.email}
    nome_utilizador: str = Field(min_length=1, max_length=100)
    nome_planta: str = Field(min_length=1, max_length=100)
    especie_id: int = Field(ge=1)
    cidade: str = Field(min_length=1, max_length=100)