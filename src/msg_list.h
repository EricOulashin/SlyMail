#ifndef SLYMAIL_MSG_LIST_H
#define SLYMAIL_MSG_LIST_H

#include "qwk.h"
#include "settings.h"
#include <functional>
#include <vector>

enum class ConfListResult
{
    Selected,
    Quit,
    OpenFile,
    RemoteSystems,
    Settings,
    Voting,
    SaveRep,
};

enum class MsgListResult
{
    ReadMessage,
    Back,
    Quit,
    NewMessage,
    Settings,
    OpenFile,
    RemoteSystems,
    SaveRep,
};

// Show the conference list
ConfListResult showConferenceList(QwkPacket& packet, int& selectedConf,
                                 Settings& settings,
                                 std::vector<QwkReply>* pendingReplies = nullptr,
                                 const std::string& baseDir = "",
                                 std::function<void()> onPendingEdited = nullptr,
                                 std::function<int(int)> getLastReadFn = nullptr);

// Show the message list for a conference (DDMsgReader-style lightbar)
// Matches DDMsgReader2 screenshot: Msg#, From, To, Subject, Date, Time columns
MsgListResult showMessageList(QwkConference& conf, int& selectedMsg,
                              Settings& settings,
                              const std::string& bbsName,
                              int lastReadMsgNum = -1,
                              std::vector<QwkReply>* pendingReplies = nullptr,
                              const std::string& baseDir = "",
                              std::function<void()> onPendingEdited = nullptr);

// Edit-pending-messages lightbar dialog. Lets the user browse and edit
// messages they've written this session. Returns true if any were modified.
// onMessageSaved is invoked once for each message that is successfully
// edited and saved (used by callers to re-write the .rep packet on disk).
bool showEditPendingMessagesDialog(std::vector<QwkReply>& pendingReplies,
                                   Settings& settings,
                                   const std::string& baseDir,
                                   std::function<void()> onMessageSaved = nullptr);

#endif // SLYMAIL_MSG_LIST_H
