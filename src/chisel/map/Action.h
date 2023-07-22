#pragma once

#include <any>

namespace chisel
{
    class Action
    {
    public:
        Action(const char *name, bool dummy)
            : m_dummy(dummy) { }

        virtual ~Action() { }

        virtual void Do() = 0;
        virtual void Undo() = 0;

        bool IsDummy() const { return m_dummy; }

        void SetGraphInfo(Action *prev, Action *next0, Action *next1)
        {
            m_prev = prev;
            m_next0 = next0;
            m_next1 = next1;
        }

        Action* GetPrev() const  { return m_prev; }
        Action* GetNext0() const { return m_next0; }
        Action* GetNext1() const  { return m_next1; }
    private:
        Action* m_prev = nullptr;
        Action* m_next0 = nullptr; // Our most recent paths.
        Action* m_next1 = nullptr; // The past do's... Only used on dummy nodes

        // Whether this is a dummy node in our history graph
        // to represent a divergence in history
        bool m_dummy = false;
    };

    // Dummy nodes to simplify the graph
    class DummyAction final : public Action
    {
    public:
        DummyAction(const char *name)
            : Action(name, true) { }

        DummyAction             (DummyAction&&) = delete;
        DummyAction& operator = (DummyAction&&) = delete;

        // Should never be executed as it's a dummy.
        void Do()   { assert(false && "Attempted to do dummy action"); }
        void Undo() { assert(false && "Attempted to undo dummy action"); }
    };

    template <typename DoCmd, typename UndoCmd>
    class FunctionAction final : public Action
    {
    public:
        FunctionAction(const char *name, DoCmd&& doCmd, UndoCmd&& undoCmd)
            : Action(name, false)
            , m_do(std::move(doCmd))
            , m_undo(std::move(undoCmd)) { }

        FunctionAction             (FunctionAction&&) = delete;
        FunctionAction& operator = (FunctionAction&&) = delete;

        void Do() { m_do(m_userdata); }
        void Undo() { m_undo(m_userdata); m_userdata.reset(); }
    private:
        std::any m_userdata;

        DoCmd   m_do;
        UndoCmd m_undo;
    };

    class ActionList
    {
    public:
        ActionList()
        {
            m_head = new DummyAction("Root");
            m_activeTail = m_head;
        }

        template <typename DoCmd, typename UndoCmd>
        void PerformAction(const char *name, DoCmd&& doFunc, UndoCmd&& undoFunc)
        {
            Action* prevTail = m_activeTail;
            Action* prevNext = prevTail->GetNext0();

            m_activeTail = new FunctionAction<DoCmd, UndoCmd>(name, std::move(doFunc), std::move(undoFunc));
            // Insert a dummy node if we have divergence.
            if (prevNext)
            {
                DummyAction *dummy = new DummyAction("Divergence");
                dummy->SetGraphInfo(prevTail, m_activeTail, prevNext);
                m_activeTail->SetGraphInfo(dummy, nullptr, nullptr);
                prevNext->SetGraphInfo(dummy, prevNext->GetNext0(), prevNext->GetNext1());
                prevTail->SetGraphInfo(prevTail->GetPrev(), m_activeTail, prevTail->GetNext1());

                //                  /-- m_activeTail (new)
                // prevTail -- dummy
                //                  \-- prevNext (prevTail->GetNext0())
            }
            else
            {
                m_activeTail->SetGraphInfo(prevTail, nullptr, nullptr);
                prevTail->SetGraphInfo(prevTail->GetPrev(), m_activeTail, prevTail->GetNext1());

                // prevTail -- m_activeTail
            }

            m_activeTail->Do();

            GraphSanityAssert();
        }

        void Undo()
        {
            if (!m_activeTail->IsDummy())
            {
                m_activeTail->Undo();

                do
                {
                    if (Action *newTail = m_activeTail->GetPrev())
                        m_activeTail = newTail;
                } while (m_activeTail->IsDummy() && m_activeTail != m_head);
            }

            GraphSanityAssert();
        }

        void Redo()
        {
            Action *redoNode = nullptr;
            do
            {
                if ( Action *nextNode = m_activeTail->GetNext0() )
                    redoNode = nextNode;
            } while ( redoNode && redoNode->IsDummy() );

            if ( redoNode && !redoNode->IsDummy() )
            {
                redoNode->Do();
                m_activeTail = redoNode;
            }

            GraphSanityAssert();
        }

        void GraphSanityAssert()
        {
            assert(m_activeTail && m_head);
            assert(!m_activeTail->IsDummy() || m_activeTail == m_head);
            assert(m_head->IsDummy());
        }
    private:
        Action *m_head = nullptr;
        Action *m_activeTail = nullptr;
    };
}
