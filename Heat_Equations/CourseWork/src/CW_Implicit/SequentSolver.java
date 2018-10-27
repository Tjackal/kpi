package CW_Implicit;

public class SequentSolver {
    private Equation equation;
    private static final double accuracy = 0.000000001;

    public SequentSolver(Equation equation) {
        this.equation = equation;
    }

    public double[][] solve() {
        final int N = equation.getN();
        final int M = equation.getM();
        final double x0 = equation.getX0();
        final double h = equation.getH();
        final double t0 = equation.getT0();
        final double tau = equation.getTau();
        double t = t0 + tau;
        double x = x0;
        double w[][] = new double[N][M];
//        double deltaW[] = new double[M];
        double deltaW[] = new double[M];
        //initial conditions
        for (int j = 0; j < M; ++j, x += h) {
            w[0][j] = equation.applyInitialCondition(x);
        }

        for (int i = 1; i < N; ++i, t += tau) {
            w[i][0] = equation.LeftBoundFunction(t);
            w[i][M - 1] = equation.RightBoundFunction(t);
            System.arraycopy(w[i - 1], 1, w[i], 1, M - 2);
            newtonSolve(w[i], w[i - 1], deltaW);
        }

        return w;
    }

    private void newtonSolve(double[] unknownSolution, double[] knownSolution, double[] deltaArray) {
        double freeTerms[] = new double[deltaArray.length];
        SeqThreeDiagonalmatrix matrix = new SeqThreeDiagonalmatrix(deltaArray.length);

        do {
            matrix.setMainDiagonalElement(0,1);
            matrix.setUpperDiagonalElement(0,0);
//          main diagonal
            for (int i = 1; i < deltaArray.length - 1; ++i) {
                matrix.setMainDiagonalElement(i, equation.MiddleDerivative(
                        unknownSolution[i - 1], unknownSolution[i], unknownSolution[i + 1]));
                freeTerms[i] = - equation.f(
                        unknownSolution[i - 1], unknownSolution[i], unknownSolution[i + 1], knownSolution[i]);
            }
//          upper diagonal
            for (int i = 1; i < deltaArray.length - 1; ++i) {
                matrix.setUpperDiagonalElement(i, equation.RightDerivative(
                        unknownSolution[i - 1], unknownSolution[i], unknownSolution[i + 1]));
            }
//          lower diagonal
            for (int i = 1; i < deltaArray.length - 1; ++i) {
                matrix.setLowerDiagonalElement(i - 1, equation.LeftDerivative(
                        unknownSolution[i - 1], unknownSolution[i], unknownSolution[i + 1]));
            }

            matrix.setMainDiagonalElement(deltaArray.length - 1,1);
            matrix.setLowerDiagonalElement(deltaArray.length - 2, 0);

            matrix.Solve(freeTerms, deltaArray);

            for (int i = 0; i < deltaArray.length; ++i) {
                unknownSolution[i] += deltaArray[i];
            }
        }
        while (!arrayApproximatelyZero(deltaArray));
    }

    private boolean arrayApproximatelyZero(double[] array) {
        for (int i = 0; i < array.length; ++i) {
            if (Math.abs(array[i]) > accuracy) {
                return false;
            }
        }
        return true;
    }


}