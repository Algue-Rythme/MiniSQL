select t1.Nom
from "database/gens.csv" t1
where t1.Age >= "18" or t1.Plat = "pizza";
