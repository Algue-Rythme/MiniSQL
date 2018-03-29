SELECT e.nom
FROM "employes.csv" e, "membres.csv" m
WHERE e.ide = m.ide
