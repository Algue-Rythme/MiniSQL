select t3.Nom, t4.Plat
    from "database/gens.csv" t3, "database/plats.csv" t4
    where t3.Plat = t4.Plat and t4.Sain <> "oui";
