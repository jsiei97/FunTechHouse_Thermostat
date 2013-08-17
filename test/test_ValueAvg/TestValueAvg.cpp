#include <QtCore>
#include <QtTest>

#include "ValueAvg.h"

class TestValueAvg : public QObject
{
    Q_OBJECT

    private:
    public:

    private slots:
        void test_ValueAvg();
};

void TestValueAvg::test_ValueAvg()
{
    ValueAvg filter;

    //Avg on the same num will always be the same
    for(int i=0; i<500; i++)
    {
        filter.addValue(10.0);
    }
    QCOMPARE(10.0, filter.getValue());

    //Test that reset works
    filter.init();
    for(int i=0; i<10; i++)
    {
        filter.addValue(5.0);
    }
    QCOMPARE(5.0, filter.getValue());


    //Test the max and min is removed
    //if there is more than X values
    filter.init();

    filter.addValue(-2.0);   //min value
    filter.addValue(400.0); //max value

    filter.addValue(13.0);
    filter.addValue(14.0);
    filter.addValue(14.0);
    filter.addValue(14.0);
    filter.addValue(15.0);

    //No 2 and 400
    //13+(14*3)+15=70/5=14
    QCOMPARE(14.0, filter.getValue());


    //Check that no values is removed if there is
    //less than 7 values
    filter.init();

    filter.addValue(5.0);
    filter.addValue(8.0);
    filter.addValue(14.0);

    //5+8+14=27/3=9
    QCOMPARE(9.0, filter.getValue());

}

QTEST_MAIN(TestValueAvg)
#include "TestValueAvg.moc"
