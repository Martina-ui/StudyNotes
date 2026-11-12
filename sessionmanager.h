#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
//the session class stores the db data we need to do all the other comparisions in the other files
//whenever the variables user_id or role need to be called, #include <sessionmanager.h>, then initiate the variables
//something like this-> int currentUserId = SessionManager::instance().userId();
class SessionManager {
public:
    static SessionManager& instance() {
        static SessionManager instance;
        return instance;
    }

    int userId() const {
        return m_userId;
    }
    QString role() const {
        return m_role;
    }
    bool isLoggedIn() const {
        return m_isLoggedIn;
    }

    bool isAdmin() const {
        return m_isLoggedIn && m_userId > 0 && m_role == "Admin";
    }

    void startSession(int id, const QString &userRole) {
        m_userId = id;
        m_role = userRole;
        m_isLoggedIn = true;
    }

    void clearSession() {
        m_userId = -1;
        m_role = "";
        m_isLoggedIn = false;
    }

private:
    SessionManager() : m_userId(-1), m_isLoggedIn(false) {} // Private constructor/deconstructor
    ~SessionManager() = default;

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    int m_userId;
    QString m_role;
    bool m_isLoggedIn;
};

#endif