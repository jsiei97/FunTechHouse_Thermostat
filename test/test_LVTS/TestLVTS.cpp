#include <QtCore>
#include <QtTest>

#include "LVTS.h"

class TestLVTS : public QObject
{
    Q_OBJECT

    private:
    public:

    private slots:
        void test_LM34();
        void test_LM34_data();

        void test_LM35();
        void test_LM35_data();

        void test_F2C();
        void test_F2C_data();
};

void TestLVTS::test_LM34_data()
{
    QTest::addColumn<int>("read");
    QTest::addColumn<double>("temperature");

    // Aref = 1.1
    // 68degF (20degC) = 0.68V
    // 1024 * (0.68V/1.1V) = 633 
    QTest::newRow("value") << 633 << 20.0;

    // 1024 * (0.50V/1.1V = 465
    QTest::newRow("value") << 465 << 10.0;
}

void TestLVTS::test_LM34()
{
    QFETCH(int, read);
    QFETCH(double, temperature);

    LVTS lvts;

    bool ok = false;

    //The value will always diff a little, so compare the rounded value...
    double value = round(lvts.lm34(read, &ok));
    if(!ok)
    {
        qDebug() << read << temperature << ok;
        QFAIL("convert failed");
    }

    QCOMPARE(temperature, value);
}

void TestLVTS::test_LM35_data()
{
    QTest::addColumn<int>("read");
    QTest::addColumn<double>("temperature");

    // Aref = 1.1
    // 20degC = 0.20V
    // 1024 * (0.20V/1.1V) = 186
    QTest::newRow("value") << 186 << 20.0;

    // 1024 * (0.30V/1.1V) = 279
    QTest::newRow("value") << 279 << 30.0;
}

void TestLVTS::test_LM35()
{
    QFETCH(int, read);
    QFETCH(double, temperature);

    LVTS lvts;

    bool ok = false;

    //The value will always diff a little, so compare the rounded value...
    double value = round(lvts.lm35(read, &ok));
    if(!ok)
    {
        qDebug() << read << temperature << ok;
        QFAIL("convert failed");
    }

    QCOMPARE(temperature, value);
}

void TestLVTS::test_F2C_data()
{
    QTest::addColumn<double>("degC");
    QTest::addColumn<double>("degF");

    QTest::newRow("value") <<  70.0 << 158.0;
    QTest::newRow("value") <<  65.0 << 149.0;
    QTest::newRow("value") <<  60.0 << 140.0;
    QTest::newRow("value") <<  55.0 << 131.0;
    QTest::newRow("value") <<  50.0 << 122.0;
    QTest::newRow("value") <<  45.0 << 113.0;
    QTest::newRow("value") <<  40.0 << 104.0;
    QTest::newRow("value") <<  35.0 <<  95.0;
    QTest::newRow("value") <<  30.0 <<  86.0;
    QTest::newRow("value") <<  25.0 <<  77.0;
    QTest::newRow("value") <<  20.0 <<  68.0;
    QTest::newRow("value") <<  15.0 <<  59.0;
    QTest::newRow("value") <<  10.0 <<  50.0;
    QTest::newRow("value") <<   5.0 <<  41.0;
    QTest::newRow("value") <<   0.0 <<  32.0;
    QTest::newRow("value") <<  -5.0 <<  23.0;
    QTest::newRow("value") << -10.0 <<  14.0;
    QTest::newRow("value") << -15.0 <<   5.0;
    QTest::newRow("value") << -20.0 <<  -4.0;
    QTest::newRow("value") << -25.0 << -13.0;
    QTest::newRow("value") << -30.0 << -22.0;
}

void TestLVTS::test_F2C()
{
    QFETCH(double, degC);
    QFETCH(double, degF);

    QCOMPARE(degC, LVTS::F2C(degF));
}


QTEST_MAIN(TestLVTS)
#include "TestLVTS.moc"
