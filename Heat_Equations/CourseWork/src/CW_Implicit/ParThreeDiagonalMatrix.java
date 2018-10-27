package CW_Implicit;

import mpi.MPI;

public class ParThreeDiagonalMatrix extends SeqThreeDiagonalmatrix {
    private double [] leftValues;
    private double [] rightValues;
    private ParalSolver solver;
    private final int rank;
    private final int numberOfProcesses;
    private final int count;

    private static final int RIGHT_COLUMN_FIRST_ELEMENT = 0;
    private static final int RIGHT_COLUMN_LAST_ELEMENT = 1;
    private static final int MAIN_DIAGONAL_FIRST_ELEMENT = 2;
    private static final int MAIN_DIAGONAL_LAST_ELEMENT = 3;
    private static final int LEFT_COLUMN_FIRST_ELEMENT = 4;
    private static final int LEFT_COLUMN_LAST_ELEMENT = 5;
    private static final int FIRST_FREE_TERM = 6;
    private static final int LAST_FREE_TERM = 7;
    private static final int UPPER_BOUND_RESULT = 8;
    private static final int LOWER_BOUND_RESULT = 9;

    public ParThreeDiagonalMatrix(int size, ParalSolver solver) {
        super(size);
        this.solver = solver;
        leftValues = new double[size];
        rightValues = new double[size];
        rank = solver.getRank();
        numberOfProcesses = solver.getNumberOfProcesses();
        count = solver.countForRank(rank);
    }

    @Override
    public void setLowerDiagonalElement(int index, double value) {
        if (index == -1)
            leftValues[0] = value;
        else
            super.setLowerDiagonalElement(index, value);
    }

    @Override
    public void setUpperDiagonalElement(int index, double value) {
        if (index == upperDiagonal.length)
            rightValues[rightValues.length - 1] = value;
        else
            super.setUpperDiagonalElement(index, value);
    }

    @Override
    public void Solve(double[] freeTerms, double[] result) {
        straightRun(freeTerms);
        reverseRun(freeTerms);
        collectBoundValuesAndSolve(freeTerms, result);
    }

    private void collectBoundValuesAndSolve(double[] freeTerms, double[] result) {
        double[] buf = new double[1];
        double upperBoundResults[] = new double[rank == 0 ? 1 : 2];
        double lowerBoundResults[] = new double[rank == numberOfProcesses - 1 ? 1 : 2];

        if (rank == 0) {

            int size = 0;
            for (int i = 0; i < numberOfProcesses; ++i) {
                size += solver.countForRank(i) == 1 ? 1 : 2;
            }

            SeqThreeDiagonalmatrix newMatrix = new SeqThreeDiagonalmatrix(size);
            double newFreeTerms[] = new double[size];
            double newResult[] = new double[size];
            int counter = 0;

            newMatrix.setMainDiagonalElement(counter, mainDiagonal[0]);
            newMatrix.setUpperDiagonalElement(counter, rightValues[0]);
            newFreeTerms[counter] = freeTerms[0];
            ++counter;

            if (count > 1) {
                newMatrix.setUpperDiagonalElement(counter, mainDiagonal[mainDiagonal.length - 1]);
                newMatrix.setMainDiagonalElement(counter, rightValues[rightValues.length - 1]);
                newMatrix.setLowerDiagonalElement(counter - 1, 0.0);
                newFreeTerms[counter] = freeTerms[freeTerms.length - 1];
                ++counter;
            }

            for (int i = 1; i < numberOfProcesses; ++i) {
                int iCount = solver.countForRank(i) - (i == numberOfProcesses - 1 ? 1 : 0);
                if (iCount > 1) {
                    MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, MAIN_DIAGONAL_FIRST_ELEMENT);
                    newMatrix.setLowerDiagonalElement(counter - 1, buf[0]);
                    MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, LEFT_COLUMN_FIRST_ELEMENT);
                    newMatrix.setMainDiagonalElement(counter, buf[0]);
                    if (i != numberOfProcesses - 1) {
                        MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, RIGHT_COLUMN_FIRST_ELEMENT);
                        newMatrix.setUpperDiagonalElement(counter, buf[0]);
                    } else {
                        newMatrix.setUpperDiagonalElement(counter, 0);
                    }
                    ++counter;
                    MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, LEFT_COLUMN_LAST_ELEMENT);
                    newMatrix.setLowerDiagonalElement(counter - 1, buf[0]);
                    if (i != numberOfProcesses - 1) {
                        MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, RIGHT_COLUMN_LAST_ELEMENT);
                        newMatrix.setMainDiagonalElement(counter, buf[0]);
                        MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, MAIN_DIAGONAL_LAST_ELEMENT);
                        newMatrix.setUpperDiagonalElement(counter, buf[0]);
                    } else {
                        MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, MAIN_DIAGONAL_LAST_ELEMENT);
                        newMatrix.setMainDiagonalElement(counter, buf[0]);
                    }
                    MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, FIRST_FREE_TERM);
                    newFreeTerms[counter - 1] = buf[0];
                    MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, LAST_FREE_TERM);
                    newFreeTerms[counter] = buf[0];
                    ++counter;
                } else if (iCount == 1) {
                    MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, MAIN_DIAGONAL_FIRST_ELEMENT);
                    newMatrix.setLowerDiagonalElement(counter - 1, buf[0]);
                    MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, LEFT_COLUMN_FIRST_ELEMENT);
                    newMatrix.setMainDiagonalElement(counter, buf[0]);
                    if (i != numberOfProcesses - 1) {
                        MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, RIGHT_COLUMN_FIRST_ELEMENT);
                        newMatrix.setUpperDiagonalElement(counter, buf[0]);
                    }
                    MPI.COMM_WORLD.Recv(buf, 0, 1, MPI.DOUBLE, i, FIRST_FREE_TERM);
                    newFreeTerms[counter] = buf[0];
                    ++counter;
                }
            }

            newMatrix.Solve(newFreeTerms, newResult);

            reorderParallelResult(newResult);

            int totalIndexDecrease = 0;
            for (int i = 1; i < numberOfProcesses; ++i) {
                int iCount = solver.countForRank(i) - (i == numberOfProcesses - 1 ? 1 : 0);
                MPI.COMM_WORLD.Isend(newResult, 2 * i - 1 - totalIndexDecrease, 2, MPI.DOUBLE, i, UPPER_BOUND_RESULT);

                if (iCount == 1) {
                    ++totalIndexDecrease;
                }

                if (i != numberOfProcesses - 1) {
                    MPI.COMM_WORLD.Isend(newResult, 2 * i + 1 - totalIndexDecrease, 2, MPI.DOUBLE, i, LOWER_BOUND_RESULT);
                } else {
                    MPI.COMM_WORLD.Isend(newResult, 2 * i + 1 - totalIndexDecrease, 1, MPI.DOUBLE, i, LOWER_BOUND_RESULT);
                }
            }

            upperBoundResults[0] = newResult[0];
            lowerBoundResults[0] = newResult[1];
            lowerBoundResults[1] = newResult[2];
        }
        else {
            if (count > 1) {
                buf[0] = mainDiagonal[0];
                MPI.COMM_WORLD.Isend(buf, 0, 1, MPI.DOUBLE, 0, MAIN_DIAGONAL_FIRST_ELEMENT);
                buf[0] = mainDiagonal[mainDiagonal.length - 1];
                MPI.COMM_WORLD.Isend(buf, 0, 1, MPI.DOUBLE, 0, MAIN_DIAGONAL_LAST_ELEMENT);
                if (rank != numberOfProcesses - 1) {
                    MPI.COMM_WORLD.Isend(rightValues, 0, 1, MPI.DOUBLE, 0, RIGHT_COLUMN_FIRST_ELEMENT);
                    MPI.COMM_WORLD.Isend(rightValues, rightValues.length - 1, 1, MPI.DOUBLE, 0,
                            RIGHT_COLUMN_LAST_ELEMENT);
                }
                MPI.COMM_WORLD.Isend(leftValues, 0, 1, MPI.DOUBLE, 0, LEFT_COLUMN_FIRST_ELEMENT);
                MPI.COMM_WORLD.Isend(leftValues, leftValues.length - 1, 1, MPI.DOUBLE, 0,
                        LEFT_COLUMN_LAST_ELEMENT);
                MPI.COMM_WORLD.Isend(freeTerms, 0, 1, MPI.DOUBLE, 0, FIRST_FREE_TERM);
                MPI.COMM_WORLD.Isend(freeTerms, freeTerms.length - 1, 1, MPI.DOUBLE, 0, LAST_FREE_TERM);
            } else if (count == 1) {
                buf[0] = mainDiagonal[0];
                MPI.COMM_WORLD.Isend(buf, 0, 1, MPI.DOUBLE, 0, MAIN_DIAGONAL_FIRST_ELEMENT);
                if (rank != numberOfProcesses - 1) {
                    MPI.COMM_WORLD.Isend(rightValues, 0, 1, MPI.DOUBLE, 0, RIGHT_COLUMN_FIRST_ELEMENT);
                }
                MPI.COMM_WORLD.Isend(leftValues, 0, 1, MPI.DOUBLE, 0, LEFT_COLUMN_FIRST_ELEMENT);
                MPI.COMM_WORLD.Isend(freeTerms, 0, 1, MPI.DOUBLE, 0, FIRST_FREE_TERM);
            }

            MPI.COMM_WORLD.Recv(upperBoundResults, 0, 2, MPI.DOUBLE, 0,
                    UPPER_BOUND_RESULT);
            if (rank != numberOfProcesses - 1) {
                MPI.COMM_WORLD.Recv(lowerBoundResults, 0, 2, MPI.DOUBLE, 0, LOWER_BOUND_RESULT);
            } else {
                MPI.COMM_WORLD.Recv(lowerBoundResults, 0, 1, MPI.DOUBLE, 0, LOWER_BOUND_RESULT);
            }
        }


        if (rank == 0) {
            result[0] = upperBoundResults[0];
            result[result.length - 2] = lowerBoundResults[0];
            result[result.length - 1] = lowerBoundResults[1];
            for (int i = 1; i < result.length - 2; ++i) {
                result[i] = (freeTerms[i]
                        - rightValues[i] * lowerBoundResults[1]) / mainDiagonal[i];
            }
        }
        else if (rank == numberOfProcesses - 1) {
            result[0] = upperBoundResults[0];
            result[1] = upperBoundResults[1];
            result[result.length - 1] = lowerBoundResults[0];
            for (int i = 1; i < result.length - 2; ++i) {
                result[i + 1] = (freeTerms[i]
                        - leftValues[i] * upperBoundResults[0]) / mainDiagonal[i];
            }
        }
        else {
            result[0] = upperBoundResults[0];
            result[1] = upperBoundResults[1];
            result[result.length - 2] = lowerBoundResults[0];
            result[result.length - 1] = lowerBoundResults[1];

            for (int i = 1; i < result.length - 3; ++i) {
                result[i + 1] = (freeTerms[i]
                        - leftValues[i] * upperBoundResults[0]
                        - rightValues[i] * lowerBoundResults[1]) / mainDiagonal[i];
            }
        }
    }

    private void reorderParallelResult(double[] result) {
        for (int i = 1; i < result.length - 1; i += 2) {
            double temp = result[i];
            result[i] = result[i + 1];
            result[i + 1] = temp;
        }
    }

    private void reverseRun(double[] freeTerms) {
        for (int i = freeTerms.length - 2; i >= 0; --i) {
            double beta = -(upperDiagonal[i] / mainDiagonal[i + 1]);
            upperDiagonal[i] = 0.0;
            freeTerms[i] += beta * freeTerms[i + 1];
            rightValues[i] += beta * rightValues[i + 1];
            leftValues[i] += beta * leftValues[i + 1];
        }
    }

    private void straightRun(double[] freeTerms) {
        for (int i = 0; i < freeTerms.length - 1; ++i) {
            double alpha = -(lowerDiagonal[i] / mainDiagonal[i]);
            lowerDiagonal[i] = 0.0;
            mainDiagonal[i + 1] += alpha * upperDiagonal[i];
            freeTerms[i + 1] += alpha * freeTerms[i];
            leftValues[i + 1] += alpha * leftValues[i];
        }
    }
}
