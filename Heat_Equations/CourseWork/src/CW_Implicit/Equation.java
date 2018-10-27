package CW_Implicit;

import mpi.MPI;


public class Equation {
    private final double x0 = 0;
    private final double x1 = 1;
    private final double h = 1.0 / 15;
    private final double t0 = 0;
    private final double t1 = 1;
    private final double tau = 1.0 / 5;
    double a = 4;
    double A = 5;
    double B = 8;
    double C = 7;

    public static void main(String[] args) {
        MPI.Init(args);
        if (MPI.COMM_WORLD.Rank() == 0) {
            long t = System.nanoTime();
            sequentialSolve();
            System.out.println();
            System.out.println("seq work time: " + ((System.nanoTime() - t) / 1000000000.0) + "s");
        }
        MPI.Finalize();
        System.out.println();
        parallelSolve(args);
        System.out.println();
    }

    private static void parallelSolve(String[] args) {
        MPI.Init(args);
        long t = System.nanoTime();
        if (MPI.COMM_WORLD.Rank() == 0)
            System.out.println("Parallel:");

        Equation equation = new Equation();
        double[][] solve = new ParalSolver(equation).solve();

        if (MPI.COMM_WORLD.Rank() == 0)
            System.out.println("par work time: " + ((System.nanoTime() - t) / 1000000000.0) + "s");
        equation.printParMatrix(solve);
        int rank = MPI.COMM_WORLD.Rank();
        int numOfProc = MPI.COMM_WORLD.Size();
        for (int i = 0; i < numOfProc; i++) {
            if (i == rank) {
                System.out.println("proc " + rank + ":");
                equation.printMatrix(solve);
            }
            MPI.COMM_WORLD.Barrier();
        }

        MPI.Finalize();
    }

    private static void sequentialSolve() {
        System.out.println("Sequential:");
        Equation equation = new Equation();
        double[][] solve = new SequentSolver(equation).solve();
        equation.printMatrix(solve);
        double exactResultMatrix[][] = new double[equation.getN()][equation.getM()];
        double t = equation.getT0();
        for (int i = 0; i < equation.getN(); ++i) {
            double x = equation.getX0();
            for (int j = 0; j < equation.getM(); ++j) {
                exactResultMatrix[i][j] = equation.exactSolution(t, x);
                x += equation.getH();
            }
            t += equation.getTau();
        }
        System.out.print("expected res:\n");
        equation.printMatrix(exactResultMatrix);
        Stats stats = new Stats(exactResultMatrix, solve);
        stats.showStats();
    }

    public double getX0() { return x0; }

    public double getH() { return h; }

    public double getT0() { return t0; }

    public double getTau() { return tau; }

    public double exactSolution(double t, double x) {
        return Math.log(Math.abs((a * Math.exp(2*(a * x + B)) + 2 * Math.exp(a * x + B) + A) / (2 * a * a * (C + t)))) - a * x - B;
    }

    public double applyInitialCondition(double x) { return exactSolution(t0, x); }

    public double LeftBoundFunction(double t) { return exactSolution(t, x0); }

    public double RightBoundFunction(double t) { return exactSolution(t, x1); }

    public double MiddleDerivative(double leftW, double curW, double rightW) {
        double sigma = tau / (h * h);
        double exponentValue = Math.exp(curW);
        double value = 1 - sigma * exponentValue * (
                - 2 + rightW - 2 * curW + leftW + Math.pow(rightW - leftW, 2) / 4)
                + a * a * tau * exponentValue;
        return value;
    }

    public double RightDerivative(double leftW, double curW, double rightW) {
        double sigma = tau / (h * h);
        double exponentValue = Math.exp(curW);
        return - sigma * exponentValue * (1 + (rightW - leftW) / 2);
    }

    public double LeftDerivative(double leftW, double curW, double rightW) {
        double sigma = tau / (h * h);
        double exponentValue = Math.exp(curW);
        return - sigma * exponentValue * (1 - (rightW - leftW) / 2);
    }

    public double f(double leftW, double curW, double rightW, double prevW) {
        double sigma = tau / (h * h);
        double exponentValue = Math.exp(curW);
        double value = curW - prevW - sigma * exponentValue * (rightW - 2 * curW + leftW
                + Math.pow(rightW - leftW, 2) / 4) + a * a * tau * exponentValue;
        return value;
    }

    public int getN() { return (int) Math.ceil((t1 - t0) / tau) + 1; }

    public int getM() { return (int) Math.ceil((x1 - x0) / h) + 1; }

    private void printMatrix(double[][] matrix) {
        for (int i = 0; i < matrix.length; ++i) {
            for (int j = 0; j < matrix[i].length; ++j) {
                System.out.print(String.format("%.4f\t", matrix[i][j]));
            }
            System.out.println();
        }
    }

    private void printParMatrix(double[][] matrix) {
        int rank = MPI.COMM_WORLD.Rank();
        int numOfProc = MPI.COMM_WORLD.Size();
        int from = (rank == 0 ? 0 : 1);
        int to = (rank == numOfProc - 1 ? matrix[0].length : matrix[0].length - 1);
        for (int i = 0; i < matrix.length; ++i) {
            for (int p = 0; p < numOfProc; p++) {
                if (p == rank) {
                    for (int j = from; j < to; ++j)
                        System.out.print(String.format("%.4f\t", matrix[i][j]));
                    if (rank == numOfProc - 1)
                        System.out.println();
                }
                MPI.COMM_WORLD.Barrier();
            }
        }
    }

}