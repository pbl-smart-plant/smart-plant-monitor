"""Chaves de cada vaso e PIN: gerar, fazer hash e comparar.
A BD so guarda a hash nunca a chave.
"""
import hashlib
import secrets

import bcrypt

def gerar_chave() -> str:
    """Chave secreta nova para um vaso: 64 chars hexadecimal 256bits aleatorios."""
    return secrets.token_hex(32)

def hash_chave(chave: str) -> str:
    """ALgoritmo SHA-256, mais rapido e eficiente que bcrypt"""

    return hashlib.sha256(chave.encode()).hexdigest()

def hash_pin(pin: str) -> str:
    """neste caso vamos usar bcrypt um pin de 6 algarismos seria facilmente descoberto com sha-256
     ao ver a bd conseguiriam descodifica-lo rapidamente,
     embora seja mais lento o bcrypt tem sal (um valor aleatorio diferente entre cada hash) """
    return bcrypt.hashpw(pin.encode(), bcrypt.gensalt()).decode()

def pin_correto(pin: str, hash_guardado: str) -> bool:
    """Faz a comparacao do pin inserido com o hash guardado"""
    try:
        return bcrypt.checkpw(pin.encode(), hash_guardado.encode())
    except ValueError:
        return False