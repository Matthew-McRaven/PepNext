#include <QTest>
#include <QtCore>

#include "sim/device/broadcast/pubsub.hpp"

class SimDevice_PubSub : public QObject {
  Q_OBJECT
private slots:
  void prod1cons0_publish() {
    auto channel =
        QSharedPointer<sim::memory::detail::Channel<quint8, quint8>>::create(0);
    auto endpoint = channel->new_endpoint();
    QVERIFY_THROWS_NO_EXCEPTION(endpoint->append_value(0x25));
  }
  void prod1cons1_publish_read() {
    auto channel =
        QSharedPointer<sim::memory::detail::Channel<quint8, quint8>>::create(0);
    auto publish = channel->new_endpoint();
    auto subscribe = channel->new_endpoint();

    // Check that we can write/read a value.
    publish->append_value(0x25);
    auto value = subscribe->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0x25);
  }

  void prod1cons1_publish_revert() {
    auto channel =
        QSharedPointer<sim::memory::detail::Channel<quint8, quint8>>::create(0);
    auto publish = channel->new_endpoint();
    auto subscribe = channel->new_endpoint();

    // Check that we can read a value from a singl producer and we can revert
    // it.
    publish->append_value(0x25);
    auto value = subscribe->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0x25);
    publish->unwrite();
    value = subscribe->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0);
  }

  void prod2cons1_publish_revert() {
    auto channel =
        QSharedPointer<sim::memory::detail::Channel<quint8, quint8>>::create(0);
    auto publish0 = channel->new_endpoint();
    auto publish1 = channel->new_endpoint();
    auto subscribe = channel->new_endpoint();
    publish0->append_value(0x25);
    publish1->append_value(0x10);

    // Read and check both values.
    auto value = subscribe->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0x25);
    value = subscribe->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0x10);

    // Check that we are reset to the root upon undoing publish0's write.
    publish0->unwrite();
    value = subscribe->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0);
  }

  void prod2cons1_publish_unread() {
    auto channel =
        QSharedPointer<sim::memory::detail::Channel<quint8, quint8>>::create(0);
    auto publish0 = channel->new_endpoint();
    auto publish1 = channel->new_endpoint();
    auto subscribe0 = channel->new_endpoint();
    publish0->append_value(0x25);
    publish1->append_value(0x10);

    // Read and check both values.
    auto value = subscribe0->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0x25);
    value = subscribe0->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0x10);

    // Check that we can unread a value.
    subscribe0->unread();
    value = subscribe0->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0x10);

    // Check that unwrite works after unread.
    publish0->unwrite();
    value = subscribe0->next_value();
    QVERIFY(value.has_value());
    QCOMPARE(*value, 0);
  }
};

#include "pubsub.test.moc"

QTEST_MAIN(SimDevice_PubSub)
