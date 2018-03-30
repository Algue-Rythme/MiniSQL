select gens.Nom, l.Langage, l.Type, l.Vitesse
from "database/languages.csv" l, "database/gens.csv" gens
where l.Langage = gens.Langage
group by l.Type
order by l.Vitesse;
