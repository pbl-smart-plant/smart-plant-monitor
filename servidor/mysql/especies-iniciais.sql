-- =====================================================================
-- PlantCare — Espécies iniciais (RF09) para a BD REAL (AWS).
-- Corre-se UMA vez, à mão, depois do primeiro arranque (Parte 4, passo 9).
-- Pode correr-se outra vez sem problema: INSERT IGNORE salta as que já existem.
--
--    Os limites são uma primeira proposta. Estes limites tem de ser verificados.
--    A luz depende do sensor escolhido (lux ou valor bruto): ajustar luz_min/luz_max.
-- =====================================================================
USE plantcare;

INSERT IGNORE INTO especies
  (nome, humidade_solo_min, humidade_solo_max, temperatura_min, temperatura_max,
   humidade_ar_min, humidade_ar_max, luz_min, luz_max)
VALUES
  -- nome                  solo %   temp °C  ar %     luz
  ('Jibóia',                30, 60,  15, 30,  40, 80,  200,  800),   -- pothos: deixar secar a camada de cima
  ('Monstera',              40, 65,  18, 30,  50, 80,  300,  900),   -- gosta de ar húmido e luz indireta
  ('Espada-de-São-Jorge',   10, 35,  15, 32,  30, 60,  100, 1000),   -- suculenta: rega pouco, aguenta pouca luz
  ('Clorofito',             35, 60,  13, 27,  40, 70,  200,  800),   -- planta-aranha
  ('Lírio-da-paz',          45, 65,  18, 27,  50, 80,  100,  500);   -- murcha quando tem sede

SELECT id, nome FROM especies ORDER BY id;
