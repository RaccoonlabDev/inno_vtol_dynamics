/**
 * @file test_vtol_dynamics.cpp
 * @author Dmitry Ponomarev
 */

#include <gtest/gtest.h>
#include <iostream>
#include <Eigen/Geometry>
#include <random>
#include <geographiclib_conversions/geodetic_conv.hpp>
#include "vtolDynamicsSim.hpp"
#include "common_math.hpp"


TEST(InnoVtolDynamicsSim, calculateWind){
    InnoVtolDynamicsSim vtolDynamicsSim;
    auto wind_mean_velocity = Eigen::Vector3d(0, 10, 0);
    double wind_variance = 0.0;
    vtolDynamicsSim.setWindParameter(wind_mean_velocity, wind_variance);
    Eigen::Vector3d expected_wind = Eigen::Vector3d(0, 10, 0);

    Eigen::Vector3d actual_wind = vtolDynamicsSim.calculateWind();
    ASSERT_EQ(actual_wind, expected_wind);
}

TEST(InnoVtolDynamicsSim, calculateAnglesOfAtack){
    InnoVtolDynamicsSim vtolDynamicsSim;
    Eigen::Vector3d airSpeed;
    double result, expected;

    std::vector<std::pair<Eigen::Vector3d, double>> dataset;
    dataset.push_back((std::make_pair(Eigen::Vector3d(0, 0, 0),     0.0)));

    dataset.push_back((std::make_pair(Eigen::Vector3d(10, 1, 1),    0.099669)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 10, 1),    0.785398)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 1, 10),    1.471128)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 2, 3),     1.2490)));

    dataset.push_back((std::make_pair(Eigen::Vector3d(-10, 1, 1),   3.041924)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(-1, 10, 1),   2.356194)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(-1, 1, 10),   1.670465)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(-1, 2, 3),    1.892547)));

    dataset.push_back((std::make_pair(Eigen::Vector3d(10, 1, -1),   -0.099669)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 10, -1),   -0.785398)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 1, -10),   -1.471128)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 2, -3),    -1.249046)));

    for(auto pair : dataset){
        result = vtolDynamicsSim.calculateAnglesOfAtack(pair.first);
        EXPECT_NEAR(result, pair.second, 0.0001);
    }
}

TEST(InnoVtolDynamicsSim, calculateAnglesOfSideslip){
    InnoVtolDynamicsSim vtolDynamicsSim;
    Eigen::Vector3d airSpeed;
    double result, expected;

    std::vector<std::pair<Eigen::Vector3d, double>> dataset;
    dataset.push_back((std::make_pair(Eigen::Vector3d(0, 0, 0),     0.0)));

    dataset.push_back((std::make_pair(Eigen::Vector3d(10, 1, 1),    0.099177)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 10, 1),    1.430307)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 1, 10),    0.099177)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 2, 3),     0.563943)));

    dataset.push_back((std::make_pair(Eigen::Vector3d(10, -1, 1),   -0.099177)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, -10, 1),   -1.430307)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, -1, 10),   -0.099177)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, -2, 3),    -0.563943)));

    dataset.push_back((std::make_pair(Eigen::Vector3d(10, 1, -1),   0.099177)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 10, -1),   1.430307)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 1, -10),   0.099177)));
    dataset.push_back((std::make_pair(Eigen::Vector3d(1, 2, -3),    0.563943)));

    for(auto pair : dataset){
        result = vtolDynamicsSim.calculateAnglesOfSideslip(pair.first);
        EXPECT_NEAR(result, pair.second, 0.001);
    }
}

TEST(CommonMath, findPrevRowIdxInIncreasingSequence){
    InnoVtolDynamicsSim vtolDynamicsSim;
    Eigen::MatrixXd table(8, 1);
    table << 5, 10, 15, 20, 25, 30, 35, 40;
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, -1),   0);
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, 10),   0);
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, 10.1), 1);
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, 15.1), 2);
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, 34.9), 5);
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, 35.1), 6);
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, 39.9), 6);
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, 40.1), 6);
    ASSERT_EQ(Math::findPrevRowIdxInIncreasingSequence(table, 50.0), 6);
}

TEST(CommonMath, findPrevRowIdxInMonotonicSequenceIncreasing){
    InnoVtolDynamicsSim vtolDynamicsSim;
    Eigen::MatrixXd table(8, 1);
    table << 5, 10, 15, 20, 25, 30, 35, 40;
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, -1),   0);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 10),   0);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 10.1), 1);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 15.1), 2);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 34.9), 5);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 35.1), 6);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 39.9), 6);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 40.1), 6);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 50.0), 6);
}

TEST(CommonMath, findPrevRowIdxInMonotonicSequenceDecreasing){
    InnoVtolDynamicsSim vtolDynamicsSim;
    Eigen::MatrixXd table(8, 1);
    table << 40, 35, 30, 25, 20, 15, 10, 5;
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, -1),   6);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 10),   5);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 10.1), 5);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 15.1), 4);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 34.9), 1);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 35.1), 0);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 39.9), 0);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 40.1), 0);
    ASSERT_EQ(Math::findPrevRowIdxInMonotonicSequence(table, 50.0), 0);
}

TEST(calculateCLPolynomial, test_normal_scalar){
    InnoVtolDynamicsSim vtolDynamicsSim;

    Eigen::MatrixXd table(2, 2);
    table << 0, 0,
             1, 1;
    double airSpeedMod = 0.5;
    Eigen::VectorXd polynomialCoeffs(1);

    ASSERT_TRUE(Math::calculatePolynomial(table, airSpeedMod, polynomialCoeffs));
    ASSERT_EQ(polynomialCoeffs[0], airSpeedMod);
}

TEST(calculateCLPolynomial, test_normal_vector){
    InnoVtolDynamicsSim vtolDynamicsSim;
    Eigen::VectorXd polynomialCoeffs(2);

    Eigen::MatrixXd table(2, 3);
    table << 0, 0, 1,
             1, 1, 2;
    double airSpeedMod = 0.5;

    Eigen::VectorXd expectedPolynomialCoeffs(2);
    expectedPolynomialCoeffs << 0.5, 1.5;

    ASSERT_TRUE(Math::calculatePolynomial(table, airSpeedMod, polynomialCoeffs));
    ASSERT_EQ(polynomialCoeffs[0], expectedPolynomialCoeffs[0]);
    ASSERT_EQ(polynomialCoeffs[1], expectedPolynomialCoeffs[1]);
}

TEST(calculateCLPolynomial, test_wrong_input_size){
    InnoVtolDynamicsSim vtolDynamicsSim;

    Eigen::MatrixXd table(1, 2);
    table << 0, 0;
    double airSpeedMod = 0.5;
    Eigen::VectorXd polynomialCoeffs(1);

    ASSERT_FALSE(Math::calculatePolynomial(table, airSpeedMod, polynomialCoeffs));
}

TEST(calculateCLPolynomial, test_wrong_table){
    InnoVtolDynamicsSim vtolDynamicsSim;

    Eigen::MatrixXd table(2, 2);
    table << 0, 0,
             0, 0;
    double airSpeedMod = 0.5;
    Eigen::VectorXd polynomialCoeffs(1);

    ASSERT_FALSE(Math::calculatePolynomial(table, airSpeedMod, polynomialCoeffs));
}

TEST(InnoVtolDynamicsSim, calculateCLPolynomial){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    Eigen::VectorXd calculatedpolynomialCoeffs(7);
    Eigen::VectorXd expectedPolynomialCoeffs(7);
    Eigen::VectorXd diff(7);
    double airSpeedMod;

    airSpeedMod = 10;
    expectedPolynomialCoeffs << -3.9340e-11, 8.2040e-09, 1.9350e-07, -3.0750e-05,
                                -4.2090e-04, 0.055200, 0.44380;
    vtolDynamicsSim.calculateCLPolynomial(airSpeedMod, calculatedpolynomialCoeffs);
    for(size_t idx = 0; idx < expectedPolynomialCoeffs.size(); idx++){
        EXPECT_NEAR(calculatedpolynomialCoeffs[idx], expectedPolynomialCoeffs[idx], 0.00001);
    }

    airSpeedMod = 0;
    expectedPolynomialCoeffs << -1.5820e-11, 8.0740e-09, 9.4100e-08, -3.1150e-05,
                                -2.8150e-04, 0.055940, 0.38260;
    vtolDynamicsSim.calculateCLPolynomial(airSpeedMod, calculatedpolynomialCoeffs);
    for(size_t idx = 0; idx < expectedPolynomialCoeffs.size(); idx++){
        EXPECT_NEAR(calculatedpolynomialCoeffs[idx], expectedPolynomialCoeffs[idx], 0.00001);
    }

    airSpeedMod = -10;
    expectedPolynomialCoeffs << 7.7000e-12, 7.9440e-09, -5.3000e-09, -3.1550e-05,
                                -1.4210e-04, 0.056680, 0.32140;
    vtolDynamicsSim.calculateCLPolynomial(airSpeedMod, calculatedpolynomialCoeffs);
    for(size_t idx = 0; idx < expectedPolynomialCoeffs.size(); idx++){
        EXPECT_NEAR(calculatedpolynomialCoeffs[idx], expectedPolynomialCoeffs[idx], 0.00001);
    }

    airSpeedMod = 45;
    expectedPolynomialCoeffs << -5.9110e-11, 7.8790e-09, 2.5740e-07, -2.9610e-05,
                                -4.8380e-04, 0.054580, 0.46370;
    vtolDynamicsSim.calculateCLPolynomial(airSpeedMod, calculatedpolynomialCoeffs);
    for(size_t idx = 0; idx < expectedPolynomialCoeffs.size(); idx++){
        EXPECT_NEAR(calculatedpolynomialCoeffs[idx], expectedPolynomialCoeffs[idx], 0.00001);
    }
}

TEST(InnoVtolDynamicsSim, calculateCSPolynomial){
    InnoVtolDynamicsSim vtolDynamicsSim;

    Eigen::VectorXd calculatedpolynomialCoeffs(7);
    double airSpeedMod = 5;

    vtolDynamicsSim.calculateCSPolynomial(airSpeedMod, calculatedpolynomialCoeffs);
}

TEST(InnoVtolDynamicsSim, calculateCDPolynomial){
    InnoVtolDynamicsSim vtolDynamicsSim;

    Eigen::VectorXd calculatedpolynomialCoeffs(7);
    double airSpeedMod = 5;

    vtolDynamicsSim.calculateCDPolynomial(airSpeedMod, calculatedpolynomialCoeffs);
    // @todo If build type is DEBUG, Eigen generates an assert
}

TEST(CommonMath, polyval){
    InnoVtolDynamicsSim vtolDynamicsSim;
    Eigen::VectorXd poly(7);
    poly << 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7;
    double value = 0.5;
    double expected_output = 3.1859;

    double actual_output = Math::polyval(poly, value);
    EXPECT_NEAR(actual_output, expected_output, 0.001);
}

TEST(InnoVtolDynamicsSim, griddata){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    Eigen::MatrixXd x(1, 3);
    Eigen::MatrixXd y(1, 4);
    Eigen::MatrixXd f(4, 3);
    double x_value;
    double y_value;
    double actual_result;
    double expected_result;

    x << 1, 2, 3;
    y << 2, 3, 4, 5;
    f << 2.5, 3.0, 3.5,
         3.0, 3.5, 4.0,
         3.5, 4.0, 4.5,
         4.0, 4.5, 5.0;

    x_value = 2.25;
    y_value = 3.75;
    expected_result = 4.0;
    actual_result = Math::griddata(x, y, f, x_value, y_value);
    EXPECT_NEAR(actual_result, expected_result, 0.001);

    x_value = 1.1;
    y_value = 4.75;
    expected_result = 3.9250;
    actual_result = Math::griddata(x, y, f, x_value, y_value);
    EXPECT_NEAR(actual_result, expected_result, 0.001);
}

TEST(InnoVtolDynamicsSim, calculateCSRudder){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);

    struct DataSet{
        double rudder_position;
        double airspeed;
        double expected;
    };
    std::vector<DataSet> data_set;
    double result;

    data_set.push_back({.rudder_position = 0,   .airspeed = 5,       .expected = -1.5009e-04});
    data_set.push_back({.rudder_position = 0,   .airspeed = 5.1,     .expected = -1.2303e-04});
    data_set.push_back({.rudder_position = 0,   .airspeed = 8.5,     .expected = 5.9762e-04});
    data_set.push_back({.rudder_position = 0,   .airspeed = 8.66025, .expected = 6.0903e-04});
    data_set.push_back({.rudder_position = 0,   .airspeed = 10,      .expected = 7.0445e-04});
    data_set.push_back({.rudder_position = 0,   .airspeed = 20,      .expected = 9.2322e-04});
    data_set.push_back({.rudder_position = 0,   .airspeed = 40,      .expected = -0.0013107});
    data_set.push_back({.rudder_position = -20, .airspeed = 5,       .expected = -0.034155});
    data_set.push_back({.rudder_position = 0,   .airspeed = 5,       .expected = -1.5009e-04});
    data_set.push_back({.rudder_position = 20,  .airspeed = 5,       .expected = 0.037053});

    for(auto test_case : data_set){
        result = vtolDynamicsSim.calculateCSRudder(test_case.rudder_position, test_case.airspeed);
        EXPECT_NEAR(result, test_case.expected, 0.001);
    }
}

TEST(InnoVtolDynamicsSim, calculateCSBeta){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);

    struct DataSet{
        double aos_degree;
        double airspeed;
        double expected;
    };
    std::vector<DataSet> data_set;
    double result;

    data_set.push_back({.aos_degree = 0,    .airspeed = 5,  .expected = -0.0032540});
    data_set.push_back({.aos_degree = 0,    .airspeed = 10, .expected = -0.0040036});
    data_set.push_back({.aos_degree = 0,    .airspeed = 15, .expected = -0.0037597});
    data_set.push_back({.aos_degree = 0,    .airspeed = 20, .expected = -0.0033221});


    for(auto test_case : data_set){
        result = vtolDynamicsSim.calculateCSBeta(test_case.aos_degree, test_case.airspeed);
        EXPECT_NEAR(result, test_case.expected, 0.0000001);
    }
}

TEST(InnoVtolDynamicsSim, DISABLED_calculateCmxAileron){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    double Cmx_aileron, airspeedNorm, aileron_pos, dynamicPressure;
    double characteristicLength = 1.5;

    airspeedNorm = 20;
    dynamicPressure = vtolDynamicsSim.calculateDynamicPressure(airspeedNorm);
    for(aileron_pos = -2e1; aileron_pos <= 2e1; aileron_pos += 4){
        Cmx_aileron = vtolDynamicsSim.calculateCmyElevator(aileron_pos, airspeedNorm);
        Cmx_aileron *= 0.5 * dynamicPressure * characteristicLength;
        std::cout << aileron_pos << " Cmx_aileron = " << Cmx_aileron << std::endl;
    }
}

TEST(InnoVtolDynamicsSim, calculateAerodynamics){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    Eigen::Vector3d Faero, Maero;

    Eigen::Vector3d airspeed(0.000001, -9.999999, 0.000001);
    double AoA = 0.958191;
    double AoS = -1.570796;
    double aileron_pos = 0.000000;
    double elevator_pos = 0.000000;
    double rudder_pos = 0.000000;

    Eigen::Vector3d extectedFaero(-4.8133e-07, 2.9513e+01, -6.0493e-06);
    Eigen::Vector3d extectedMaero(0.21470, 0.69480, -0.31633);

    vtolDynamicsSim.calculateAerodynamics(airspeed, AoA, AoS, aileron_pos, elevator_pos, rudder_pos,
                                          Faero, Maero);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(Faero[idx], extectedFaero[idx], 0.001);
        EXPECT_NEAR(Maero[idx], extectedMaero[idx], 0.001);
    }
}

TEST(InnoVtolDynamicsSim, calculateAerodynamicsCaseAileron){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    Eigen::Vector3d expectedResult, Faero, Maero;

    Eigen::Vector3d airspeed(5, 5, 5);
    double AoA = 0.1;
    double AoS = 0.1;
    double aileron_pos = 0.5;
    double elevator_pos = 0.0;
    double rudder_pos = 0.0;

    Eigen::Vector3d extectedFaero(7.4133, -4.3077, -6.6924);
    Eigen::Vector3d extectedMaero(0.333818, 1.754507, -0.037038);

    vtolDynamicsSim.calculateAerodynamics(airspeed, AoA, AoS, aileron_pos, elevator_pos, rudder_pos,
                                          Faero, Maero);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(Faero[idx], extectedFaero[idx], 0.02);
        EXPECT_NEAR(Maero[idx], extectedMaero[idx], 0.02);
    }
}

TEST(InnoVtolDynamicsSim, calculateAerodynamicsCaseElevator){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    Eigen::Vector3d expectedResult, Faero, Maero;

    Eigen::Vector3d airspeed(5, 5, 5);
    double AoA = 0.1;
    double AoS = 0.1;
    double aileron_pos = 0.0;
    double elevator_pos = 5;
    double rudder_pos = 0.0;

    Eigen::Vector3d extectedFaero(7.4133, -4.3077, -6.6924);
    Eigen::Vector3d extectedMaero(0.190243, 1.220935, -0.037038);

    vtolDynamicsSim.calculateAerodynamics(airspeed, AoA, AoS, aileron_pos, elevator_pos, rudder_pos,
                                          Faero, Maero);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(Faero[idx], extectedFaero[idx], 0.02);
        EXPECT_NEAR(Maero[idx], extectedMaero[idx], 0.02);
    }
}

TEST(InnoVtolDynamicsSim, calculateAerodynamicsAoA){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    Eigen::Vector3d diff, expectedResult, Faero, Maero;

    Eigen::Vector3d airspeed(5, 5, 5);
    double AoA = 27.0 * 3.1415 / 180.0;
    double AoS = 0;
    double aileron_pos = 0.0;
    double elevator_pos = 0.0;
    double rudder_pos = 0.0;

    Eigen::Vector3d extectedFaero(6.0625, -7.7260, -17.5536);
    Eigen::Vector3d extectedMaero(0.16512, 1.26568, -0.11093);

    vtolDynamicsSim.calculateAerodynamics(airspeed, AoA, AoS, aileron_pos, elevator_pos, rudder_pos,
                                          Faero, Maero);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(Faero[idx], extectedFaero[idx], 0.04);
        EXPECT_NEAR(Maero[idx], extectedMaero[idx], 0.04);
    }
}

TEST(InnoVtolDynamicsSim, calculateAerodynamicsRealCase){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    Eigen::Vector3d expectedResult, Faero, Maero;

    Eigen::Vector3d airspeed(2.93128, 0.619653, 0.266774);
    double AoA = 45 * 3.1415 / 180.0;
    double AoS = 11.8888 * 3.1415 / 180.0;
    double aileron_pos = 0.0;
    double elevator_pos = 0.0;
    double rudder_pos = 0.0;

    Eigen::Vector3d extectedFaero(-2.28665, -0.92928, -2.66499);
    Eigen::Vector3d extectedMaero(0.017652, 0.074924, -0.024468);

    vtolDynamicsSim.calculateAerodynamics(airspeed, AoA, AoS, aileron_pos, elevator_pos, rudder_pos,
                                          Faero, Maero);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(Faero[idx], extectedFaero[idx], 0.04);
        EXPECT_NEAR(Maero[idx], extectedMaero[idx], 0.04);
    }
}

TEST(thruster, thrusterFirstZeroCmd){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    double control,
           actualThrust, actualTorque, actualRpm,
           expectedThrust, expectedTorque, expectedRpm;

    control = 0;
    expectedThrust = 0;
    expectedTorque = 0;
    expectedRpm = 0;
    vtolDynamicsSim.thruster(control, actualThrust, actualTorque, actualRpm);
    EXPECT_NEAR(actualThrust, expectedThrust, 0.001);
    EXPECT_NEAR(actualTorque, expectedTorque, 0.00001);
    EXPECT_NEAR(actualRpm, expectedRpm, 0.00001);
}
TEST(thruster, thrusterSecond){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    double control,
           actualThrust, actualTorque, actualRpm,
           expectedThrust, expectedTorque, expectedRpm;

    control = 134.254698;
    expectedThrust = 3.590800;
    expectedTorque = 0.013696;
    expectedRpm = 732.298;
    vtolDynamicsSim.thruster(control, actualThrust, actualTorque, actualRpm);
    EXPECT_NEAR(actualThrust, expectedThrust, 0.0001);
    EXPECT_NEAR(actualTorque, expectedTorque, 0.000001);
    EXPECT_NEAR(actualRpm, expectedRpm, 0.001);
}
TEST(thruster, thrusterThird){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    double control,
           actualThrust, actualTorque, actualRpm,
           expectedThrust, expectedTorque, expectedRpm;

    control = 500.004648;
    expectedThrust = 15.8930;
    expectedTorque = 0.27273;
    expectedRpm = 2727.3;
    vtolDynamicsSim.thruster(control, actualThrust, actualTorque, actualRpm);
    EXPECT_NEAR(actualThrust, expectedThrust, 0.001);
    EXPECT_NEAR(actualTorque, expectedTorque, 0.00001);
    EXPECT_NEAR(actualRpm, expectedRpm, 0.2);
}

/**
 * @note In InnoDynamics the altitude is directed to the bottom, but in this simulator
 * it is directed to the top, so we perform invertion.
 */
void calculateNewState(double dt,
                    std::vector<double> actuators,
                    Eigen::Vector3d Maero,
                    Eigen::Vector3d Faero,
                    Eigen::Vector3d initialLinearVelocity,
                    Eigen::Vector3d initialAngularVelocity,
                    Eigen::Vector3d initialPosition,
                    Eigen::Quaterniond initialAttitude,
                    Eigen::Vector3d& expectedAngAccel,
                    Eigen::Vector3d& expectedLinAccel,
                    Eigen::Vector3d& angularAcceleration,
                    Eigen::Vector3d& linearAcceleration){
    InnoVtolDynamicsSim vtolDynamicsSim;
    ASSERT_EQ(vtolDynamicsSim.init(), 0);
    vtolDynamicsSim.setInitialVelocity(initialLinearVelocity, initialAngularVelocity);
    vtolDynamicsSim.setInitialPosition(initialPosition, initialAttitude);

    vtolDynamicsSim.calculateNewState(Maero, Faero, actuators, dt);
    angularAcceleration = vtolDynamicsSim.getAngularAcceleration();
    linearAcceleration = vtolDynamicsSim.getLinearAcceleration();
}

TEST(InnoVtolDynamicsSim, calculateNewStateFirstCaseOnlyAttitude){
    double dt = 0.002500;
    std::vector<double> actuators{0, 0, 0, 0, 0, 0, 0, 0};
    Eigen::Quaterniond initAttitude(1, 0.2, 0.10, 0.05);
    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(0.0, 0.0, 0.0),
                    Maero(0.0, 0.0, 0.0),
                    initialLinVel(0, 0, 0),
                    initAngVel(0, 0, 0),
                    initPose(0, 0, 0),
                    expectedAngAccel(0.0, 0.0, 0.0),
                    expectedLinAccel(2.5377e-16, -5.0753e-16, 9.8066e+00);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 1e-04);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 1e-04);
    }
}

TEST(InnoVtolDynamicsSim, calculateNewStateSecondCaseOnlyAngularVelocity){
    double dt = 0.002500;
    std::vector<double> actuators{0, 0, 0, 0, 0, 0, 0, 0};
    Eigen::Quaterniond initAttitude(1, 0.00, 0.00, 0.00);
    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(0.0, 0.0, 0.0),
                    Maero(0.0, 0.0, 0.0),
                    initialLinVel(0, 0, 0),
                    initAngVel(0.3, 0.2, 0.1),
                    initPose(0, 0, 0),
                    expectedAngAccel(-1.9719e-02,   2.9589e-02,     -8.3459e-04),
                    expectedLinAccel(9.9127e-19,    1.9825e-18,     9.8066e+00);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 6e-05);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 6e-05);
    }
}

TEST(InnoVtolDynamicsSim, calculateNewStateThirdCaseOnlyFaero){
    double dt = 0.002500;
    std::vector<double> actuators{0, 0, 0, 0, 0, 0, 0, 0};
    Eigen::Quaterniond initAttitude(1, 0.00, 0.00, 0.00);
    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(5.7448e-01, 2.9513e+01, 6.1333e-01),
                    Maero(0.0, 0.0, 0.0),
                    initialLinVel(0, 0, 0),
                    initAngVel(0.0, 0.0, 0.0),
                    initPose(0, 0, 0),
                    expectedAngAccel(0.0,       0.0,        0.0),
                    expectedLinAccel(0.082069,  4.216143,   9.894269);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 6e-05);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 6e-05);
    }
}

TEST(InnoVtolDynamicsSim, calculateNewStateFourthCaseOnlyMaero){
    double dt = 0.002500;
    std::vector<double> actuators{0, 0, 0, 0, 0, 0, 0, 0};
    Eigen::Quaterniond initAttitude(1, 0.00, 0.00, 0.00);
    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(0.0, 0.0, 0.0),
                    Maero(-0.214696, -0.694801, -0.316328),
                    initialLinVel(0, 0, 0),
                    initAngVel(0.0, 0.0, 0.0),
                    initPose(0, 0, 0),
                    expectedAngAccel(-0.34251,      -1.07821,       -0.25057),
                    expectedLinAccel(7.7443e-21,    -3.8722e-21,    9.8066e+00);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 6e-05);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 6e-05);
    }
}

TEST(InnoVtolDynamicsSim, calculateNewStateFifthCaseOnlyCopterMotorsWithEqualPower){
    double dt = 0.002500;
    std::vector<double> actuators{700, 700, 700, 700, 0, 0, 0, 0};
    Eigen::Quaterniond initAttitude(1, 0.00, 0.00, 0.00);
    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(0.0, 0.0, 0.0),
                    Maero(0.0, 0.0, 0.0),
                    initialLinVel(0, 0, 0),
                    initAngVel(0.0, 0.0, 0.0),
                    initPose(0, 0, 0),
                    expectedAngAccel(0.00000,       0.00000,        0.00000),
                    expectedLinAccel(0.00000,       0.00000,        -6.36769);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 6e-05);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 6e-05);
    }
}

TEST(InnoVtolDynamicsSim, calculateNewStateSixthCaseOnlyCopterMotorsWithNotEqualPower){
    double dt = 0.002500;
    std::vector<double> actuators{700, 680, 660, 640, 0, 0, 0, 0};
    Eigen::Quaterniond initAttitude(1, 0.00, 0.00, 0.00);
    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(0.0, 0.0, 0.0),
                    Maero(0.0, 0.0, 0.0),
                    initialLinVel(0, 0, 0),
                    initAngVel(0.0, 0.0, 0.0),
                    initPose(0, 0, 0),
                    expectedAngAccel(0.1354,        1.2944,         0.10723),
                    expectedLinAccel(-1.3753e-04,   1.2938e-05,     -5.0505e+00);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 6e-05);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 6e-05);
    }
}

TEST(InnoVtolDynamicsSim, calculateNewStateSeventhCaseOnlyICE){
    double dt = 0.002500;
    std::vector<double> actuators{0, 0, 0, 0, 500, 0, 0, 0};
    Eigen::Quaterniond initAttitude(1, 0.00, 0.00, 0.00);
    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(0.0, 0.0, 0.0),
                    Maero(0.0, 0.0, 0.0),
                    initialLinVel(0, 0, 0),
                    initAngVel(0.0, 0.0, 0.0),
                    initPose(0, 0, 0),
                    expectedAngAccel(-0.43508,      0.00000,        0.00000),
                    expectedLinAccel(2.2705e+00,    3.8722e-21,     9.8066e+00);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 6e-05);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 6e-05);
    }
}

TEST(InnoVtolDynamicsSim, calculateNewStateEightComplexWithoutInitialAttitude){
    double dt = 0.002500;
    std::vector<double> actuators{600, 550, 450, 500, 650, 0, 0, 0};
    Eigen::Quaterniond initAttitude(1, 0, 0, 0);

    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(15.0, 10.0, 5.0),
                    Maero(5.0, 10.0, 15.0),
                    initialLinVel(15, 3, 1),
                    initAngVel(0.5, 0.4, 0.3),
                    initPose(0, 0, 10),
                    expectedAngAccel(5.1203, 16.15784, 11.9625),
                    expectedLinAccel(5.60908, 1.44474, 0.80233);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 1e-03);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 1e-03);
    }
}

TEST(InnoVtolDynamicsSim, calculateNewStateEightComplexFull){
    double dt = 0.002500;
    std::vector<double> actuators{600, 550, 450, 500, 650, 4, 7, 11};
    Eigen::Quaterniond initAttitude(0.9833, 0.1436, 0.106, 0.03427);

    Eigen::Vector3d angAccel, linAccel, diff,
                    Faero(15.0, 10.0, 5.0),
                    Maero(5.0, 10.0, 15.0),
                    initialLinVel(15, 3, 1),
                    initAngVel(0.5, 0.4, 0.3),
                    initPose(0, 0, 10),
                    expectedAngAccel(5.1202, 16.15784, 11.9625),
                    expectedLinAccel(3.45031, 4.40765, 0.68005);

    calculateNewState(dt, actuators,  Maero, Faero,
                      initialLinVel, initAngVel, initPose, initAttitude,
                      expectedAngAccel, expectedLinAccel,
                      angAccel, linAccel);

    for(size_t idx = 0; idx < 3; idx++){
        EXPECT_NEAR(angAccel[idx], expectedAngAccel[idx], 1e-03);
        EXPECT_NEAR(linAccel[idx], expectedLinAccel[idx], 1e-03);
    }
}


int main(int argc, char *argv[]){
    testing::InitGoogleTest(&argc, argv);
    ros::init(argc, argv, "tester");
    return RUN_ALL_TESTS();
}
