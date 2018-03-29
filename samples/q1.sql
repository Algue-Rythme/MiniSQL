SELECT e.nom AS emp, d.nom AS dpt
FROM "employes.csv" e, "departements.csv" d
WHERE e.dpt = d.idd
