TEMPLATE = app
CONFIG += console
CONFIG += C++14
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

SOURCES += \
    task_7/task_7_o.cpp
#    seminar02.09.16.cpp

HEADERS += \
    graph.h \
    flow.h \
    barrier.h \
    sgtl.h \
    sgtl2.h \
    sgtl2.1.h \
    task_5/thread_safe_queue.h \
    task_3/barrier.h \
    task_5/thread_pool.h \
    task_5/parallel_sort.h \
    task_6/sgtl-strings.h \
    task_7/lock_free_queue(A).h \
    task_7/lock_free_queue.h \
    exam/atomic_max.h \
    exam/recursive_mutex.h \
    exam/sorted_list.h \
    exam/thread_safe_queue.h \
    sgtl-graphs.h \
    sgtl-strings.h

