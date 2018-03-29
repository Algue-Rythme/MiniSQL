SELECT p.titre, e.nom
FROM "employes.csv" e, "projets.csv" p, "membres.csv" m
WHERE e.ide = m.ide
  AND m.idp = p.idp
