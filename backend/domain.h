#ifndef DOMAIN_H
#define DOMAIN_H

#include <QString>

enum class Role {
    User,
    Admin
};

enum class Tables {
    unknown,
    admins,
    instruments,
    instrument_types,
    clients,
    purchases
};

inline Tables StringToTables(const QString& table){
    if (table == "admins"){
        return Tables::admins;
    }
    else if (table == "instruments"){
        return Tables::instruments;
    }
    else if (table == "instrument_types"){
        return Tables::instrument_types;
    }
    else if (table == "clients"){
        return Tables::clients;
    }
    else if (table == "purchases"){
        return Tables::purchases;
    }
    else{
        return Tables::unknown;
    }
}

inline QString TablesToString(const Tables& table){
    switch(table){
        case Tables::admins:
            return "admins";
        case Tables::instruments:
            return "instruments";
        case Tables::instrument_types:
            return "instrument_types";
        case Tables::clients:
            return "clients";
        case Tables::purchases:
            return "purchases";
        default:
            return "unknown";
    }
}

inline Role StringToRole(const QString& role) {
    if (role == "admin"){
        return Role::Admin;
    }
    else {
        return Role::User;
    }
}

#endif // DOMAIN_H
