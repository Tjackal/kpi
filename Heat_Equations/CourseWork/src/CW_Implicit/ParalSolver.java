package CW_Implicit;

import mpi.MPI;

import java.util.stream.IntStream;

public class ParalSolver {
    private final Equation equation;
    private final int rank;
    private final int fromIndex;
    private final int count;
    private final int numberOfProcesses;
    private final int N;
    private final int M;
    private static final int ARRAY_APPROXIMATELY_ZERO = 10;
    private static final double accuracy = 0.000000001;

    public ParalSolver(Equation equation) {
        this.equation = equation;
        N = equation.getN();
        M = equation.getM();

        rank = MPI.COMM_WORLD.Rank();
        numberOfProcesses = MPI.COMM_WORLD.Size();
        fromIndex = fromIndexOfRank(rank);
        count = countForRank(rank);
    }

    public double[][] solve() {
        double [][] w = new double[N][count];
        double [] deltaW = new double[count];
        double h = equation.getH();
        double tau = equation.getTau();
        double t = equation.getT0() + tau;

        for (int j = 0; j < count; ++j)
            w[0][j] = equation.applyInitialCondition(h * (fromIndex + j));
        for (int i = 1; i < N; ++i) {
            if (rank == 0)
                w[i][0] = equation.LeftBoundFunction(t);
            else if (rank == numberOfProcesses - 1)
                w[i][count - 1] = equation.RightBoundFunction(t);
            int offset = (rank == 0 ? 1 : 0);
            System.arraycopy(w[i - 1], offset, w[i], offset,
                    rank == numberOfProcesses - 1 ? w[i].length - 1 : w[i].length - offset);
            parallelNewtonSolve(w[i], w[i - 1], deltaW);
            t += tau;
        }
        return w;
    }

    private void parallelNewtonSolve(double[] unknownSolution, double[] knownSolution, double[] deltaArray) {
        double freeTerms[] = new double[deltaArray.length - (rank == 0 || rank == numberOfProcesses - 1 ? 1 : 2)];
        int startOffset = rank == 0 ? 1 : 0;
        int lastOffset = rank == 0 ? 1 : 2;

        do {
            SeqThreeDiagonalmatrix matrix = new ParThreeDiagonalMatrix(deltaArray.length - (rank == 0 || rank == numberOfProcesses - 1 ? 1 : 2), this);
            if (rank == 0) {
                matrix.setMainDiagonalElement(0, 1);
                matrix.setUpperDiagonalElement(0, 0);
                freeTerms[0] = 0;
            }

           // for (int i = startOffset; i < count - lastOffset; ++i) {
            IntStream.range(startOffset, count - lastOffset).parallel().forEach(i ->{
                matrix.setMainDiagonalElement(i, equation.MiddleDerivative(unknownSolution[i - startOffset],
                        unknownSolution[i + 1 - startOffset], unknownSolution[i + 2 - startOffset]));
                matrix.setUpperDiagonalElement(i, equation.RightDerivative(unknownSolution[i - startOffset],
                        unknownSolution[i + 1 - startOffset], unknownSolution[i + 2 - startOffset]));
                matrix.setLowerDiagonalElement(i - 1, equation.LeftDerivative(unknownSolution[i - startOffset],
                        unknownSolution[i + 1 - startOffset], unknownSolution[i + 2 - startOffset]));
                freeTerms[i] = - equation.f(unknownSolution[i - startOffset], unknownSolution[i + 1 - startOffset],
                        unknownSolution[i + 2 - startOffset], knownSolution[i + 1 - startOffset]);
            });
            if (rank == numberOfProcesses - 1) {
                matrix.setMainDiagonalElement(freeTerms.length - 1,1);
                matrix.setLowerDiagonalElement(freeTerms.length - 2, 0);
                freeTerms[freeTerms.length - 1] = 0;
            }
            matrix.Solve(freeTerms, deltaArray);
            for (int i = 0; i < deltaArray.length; ++i) {
                unknownSolution[i] += deltaArray[i];
            }
            MPI.COMM_WORLD.Barrier();
        }
        while (!parallelArrayApproximatelyZero(deltaArray));
    }

    private boolean parallelArrayApproximatelyZero(double[] array) {
        boolean[] result = {sequentialArrayApproximatelyZero(array)};
        if (rank == 0) {
            for (int i = 1; i < numberOfProcesses && result[0]; ++i) {
                MPI.COMM_WORLD.Recv(result, 0,1,MPI.BOOLEAN, i, ARRAY_APPROXIMATELY_ZERO);
            }

            for (int i = 1; i < numberOfProcesses; ++i) {
                MPI.COMM_WORLD.Isend(result, 0,1,MPI.BOOLEAN, i, ARRAY_APPROXIMATELY_ZERO);
            }
        }
        else {
            MPI.COMM_WORLD.Isend(result, 0,1,MPI.BOOLEAN,0, ARRAY_APPROXIMATELY_ZERO);
            MPI.COMM_WORLD.Recv(result,0,1, MPI.BOOLEAN, 0, ARRAY_APPROXIMATELY_ZERO);
        }
        return result[0];
    }

    private boolean sequentialArrayApproximatelyZero(double[] array) {
        for (double item : array) {
            if (Math.abs(item) > accuracy) {
                return false;
            }
        }

        return true;
    }

    public int fromIndexOfRank(int rank) {
        int reminder = M % numberOfProcesses;
        return rank * (M / numberOfProcesses) +
                (rank < reminder ? rank : reminder) + (rank == 0 ? 0 : -1);
    }

    public int countForRank(int rank) {
        int reminder = M % numberOfProcesses;
        return (rank == 0 ? 0 : 1) + (M / numberOfProcesses) + (rank < reminder ? 1 : 0) + (rank == numberOfProcesses - 1 ? 0 : 1);
    }

    public int getRank() {
        return rank;
    }

    public int getNumberOfProcesses() {
        return numberOfProcesses;
    }
}
