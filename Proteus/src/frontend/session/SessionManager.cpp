#include <SessionManager.h>
#include <PluginManager.h>

using namespace NS_Proteus;

SessionManager::SessionManager(Logger* l) : logger(l) {}

bool SessionManager::Initialize(const SessionManagerConfig& cfg) {
    this->config = cfg;
    return initialized = true;
}

void SessionManager::Shutdown() {
    ClearAllSessions();
    initialized = false;
    activeSessionID = 0;
    nextSessionID = 1;
    lastError.clear();
}

SessionResult SessionManager::CreateSession(ConsoleID console, SessionID& outID) {
    SessionResult r;

    // ensure manager is initialized
    if (!initialized) {
        r = INTERNAL_ERROR_SESSION_RESULT;
        r.message = lastError = "CreateSession() failed: SessionManager not initialized.";
        return r;
    }

    // ensure valid console type
    if (console == ConsoleID::NONE) {
        r = INVALID_ARGUMENT_SESSION_RESULT;
        r.message = lastError = "CreateSession() failed: No console selected.";
        return r;
    }

    // ensure console type is available
    if (!PluginManager::IsConsoleAvailable(console)) {
        r = UNSUPPORTED_CONSOLE_SESSION_RESULT;
        r.message = lastError = "CreateSession() failed: Selected console is unavailable.";
        return r;
    }

    // ensure session id is available
    while (sessions.contains(nextSessionID)) {
        nextSessionID++;
        // session id 0 is reserved and invalid; also this prevents overflow if someone decides
        // to be psychotic and try to create over four billion sessions
        if (nextSessionID == 0) {
            r = INTERNAL_ERROR_SESSION_RESULT;
            r.message = lastError = "CreateSession() failed: No more session slots available.";
            return r;
        }
    }
    SessionID id = nextSessionID;

    // allocate ConsoleSession object
    uptr<ConsoleSession> consoleSession;
    try {
      consoleSession = make_unique<ConsoleSession>(logger);
    } catch (const std::bad_alloc&) {
        r = INTERNAL_ERROR_SESSION_RESULT;
        r.message = lastError = "CreateSession() failed: Could not allocate memory for ConsoleSession.";
        return r;
    }

    // ensure success of internal CreateSession call
    r = consoleSession->CreateSession(console);
    if (!r.success) {
        lastError = "CreateSession() failed:\n\t" + r.message;
        return r;
    }

    // insert slot into map and order list
    bool inserted = false;
    try {
        Session slot{id, console, move(consoleSession) };

        auto [it, ok] = sessions.emplace(id, std::move(slot));
        if (!ok) {
            r = INTERNAL_ERROR_SESSION_RESULT;
            r.message = lastError = "CreateSession() failed: Session ID collision during insertion.";
            return r;
        }
        inserted = true;

        order.push_back(id);
    } catch (...) {
        if (inserted) sessions.erase(id);
        r = INTERNAL_ERROR_SESSION_RESULT;
        r.message = lastError = "CreateSession() failed: Could not insert new session into manager.";
        return r;
    }

    // set active session if we don't already have one
    if (!HasActiveSession()) activeSessionID = id;

    // update outID
    outID = id;

    // increment nextSessionID to point at next slot
    nextSessionID++;

    // creation succeeded; clear last error
    lastError.clear();
    return r;
}

SessionResult SessionManager::DestroySession(SessionID id) {
    SessionResult r;

    // try to find the session
    auto it = sessions.find(id);

    // if it doesn't exist, return error
    if (it == sessions.end()) {
        r = INVALID_ARGUMENT_SESSION_RESULT;
        r.message = lastError = "DestroySession() failed: session does not exist.";
        return r;
    }
    
    // otherwise, return result of attempted shutdown
    r = it->second.session->Shutdown();
    if (!r.success) lastError = "Shutdown() failed: " + r.message;
    else {
        sessions.erase(id);
        order.erase(remove(order.begin(), order.end(), id), order.end());
        PromoteSession();
        lastError.clear();
    }
    return r;
}

SessionResult SessionManager::LoadROM(SessionID id, const path& romPath, const string& romName) {
    SessionResult r;

    // try to find the session
    auto it = sessions.find(id);

    // if it doesn't exist, return error
    if (it == sessions.end()) {
        r = INVALID_ARGUMENT_SESSION_RESULT;
        r.message = lastError = "LoadROM() failed: session does not exist.";
        return r;
    }
    
    // otherwise, return result of attempted rom load
    r = it->second.session->LoadROM(romPath, romName);
    if (!r.success) lastError = "LoadROM() failed: " + r.message;
    else lastError.clear();
    return r;
}

SessionResult SessionManager::Start(SessionID id) {
    SessionResult r;

    // try to find the session
    auto it = sessions.find(id);

    // if it doesn't exist, return error
    if (it == sessions.end()) {
        r = INVALID_ARGUMENT_SESSION_RESULT;
        r.message = lastError = "Start() failed: session does not exist.";
        return r;
    }

    r = it->second.session->Start();
    if (!r.success) lastError = "Start() failed: " + r.message;
    else lastError.clear();
    return r;
}

SessionResult SessionManager::Pause(SessionID id) {
    SessionResult r;

    auto it = sessions.find(id);

    if (it == sessions.end()) {
        r = INVALID_ARGUMENT_SESSION_RESULT;
        r.message = lastError = "Pause() failed: session does not exist.";
        return r;
    }

    r = it->second.session->Pause();
    if (!r.success) lastError = "Pause() failed: " + r.message;
    else lastError.clear();
    return r;
}

SessionResult SessionManager::Reset(SessionID id) {
    SessionResult r;

    auto it = sessions.find(id);

    if (it == sessions.end()) {
        r = INVALID_ARGUMENT_SESSION_RESULT;
        r.message = lastError = "Reset() failed: session does not exist.";
        return r;
    }

    r = it->second.session->Reset();
    if (!r.success) lastError = "Reset() failed: " + r.message;
    else lastError.clear();
    return r;
}

bool SessionManager::SetActiveSession(SessionID id) {
    auto it = sessions.find(id);

    if (it == sessions.end()) {
        lastError = "SetActiveSession() failed: session does not exist.";
        return false;
    }

    activeSessionID = id;
    lastError.clear();
    return true;
}

bool SessionManager::HasSession(SessionID id) const {
    auto it = sessions.find(id);

    if (it == sessions.end()) return false;

    return true;
}

ConsoleSession* SessionManager::GetSession(SessionID id) {
    return const_cast<ConsoleSession*>(
        std::as_const(*this).GetSession(id)
    );
}

const ConsoleSession* SessionManager::GetSession(SessionID id) const {
    auto it = sessions.find(id);
    
    if (it == sessions.end()) return nullptr;

    return it->second.session.get();
}

ConsoleSession* SessionManager::GetActiveSession() {
    return const_cast<ConsoleSession*>(
        std::as_const(*this).GetActiveSession()
    );
}

const ConsoleSession* SessionManager::GetActiveSession() const {
    if (activeSessionID == 0) return nullptr;

    return GetSession(activeSessionID);
}

vector<SessionInfo> SessionManager::ListSessions() const {
    vector<SessionInfo> info;

    for (const SessionID id : order) {
        const Session& ses = sessions.at(id);
        info.push_back(SessionInfo{
            .id = id,
            .console = ses.console,
            .state = ses.session->GetState(),
            .romName = ses.romName(),
            .romPath = ses.romPath(),
            .active = id == activeSessionID
        });
    }

    return info;
}

void SessionManager::ClockSessions() {
    if (!initialized) return;

    switch (config.scheduleMode) {
        default:
        case ScheduleMode::SINGLE:
            {
                ConsoleSession* s = GetActiveSession();
                if (s != nullptr && s->GetState() == ConsoleSessionState::RUNNING) {
                    IConsole* c = s->GetConsole().get();
                    if (c) c->clock();
                }
            }
            break;
        case ScheduleMode::MULTI:
            {
                for (const auto& [_, session] : sessions) {
                    if (session.session->GetState() == ConsoleSessionState::RUNNING) {
                        IConsole* c = session.session->GetConsole().get();
                        if (c) c->clock();
                    }
                }
            }
            break;
    }
}

sptr<IConsole> SessionManager::GetActiveConsole() const {
    const ConsoleSession* s = GetActiveSession();
    if (s == nullptr) return nullptr;
    return s->GetConsole();
}

sptr<IDebugger> SessionManager::GetActiveDebugger() const {
    const ConsoleSession* s = GetActiveSession();
    if (s == nullptr) return nullptr;
    return s->GetDebugger();
}

const SessionManager::Session* SessionManager::FindSlot(SessionID id) const {
    auto it = sessions.find(id);

    if (it == sessions.end()) return nullptr;

    return &it->second;
}

SessionManager::Session* SessionManager::FindSlot(SessionID id) {
    return const_cast<Session*>(
        std::as_const(*this).FindSlot(id)
    );
}

void SessionManager::ClearAllSessions() {
    vector<SessionID> ids;
    ids.reserve(sessions.size());
    for (const auto& [id, _] : sessions) ids.push_back(id);

    for (SessionID id : ids) DestroySession(id);
}

void SessionManager::PromoteSession() {
    if (activeSessionID != 0 && sessions.contains(activeSessionID)) return;

    for (const SessionID id : order) {
        if (sessions.contains(id)) {
            activeSessionID = id;
            return;
        }
    }

    activeSessionID = 0;
}