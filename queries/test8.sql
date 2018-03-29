select gens.Nom, gens.Age, gens.Taille
from "database/gens.csv" gens
where gens.Nom=gens.Nom
order by gens.Age desc, gens.Taille asc;
