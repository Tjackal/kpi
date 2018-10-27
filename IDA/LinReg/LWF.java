import Jama.Matrix;

public class LWF {
    public static void main(String[] args) {

        double y [][] = {
                {61.6},
                {86.1},
                {71.8},
                {80.9},
                {60.2},
                {65.2},
                {64.6},
                {81.6},
                {92.3},
                {84.6},
                {74.3},
                {92.9},
                {48.9}
        };
        Matrix Y = new Matrix(y);
        norm(Y);
        double x [][] = {
                {1, 162, 25},
                {1, 178, 44},
                {1, 166, 33},
                {1, 180, 22},
                {1, 154, 50},
                {1, 166, 24},
                {1, 176, 49},
                {1, 180, 68},
                {1, 190, 23},
                {1, 176, 47},
                {1, 174, 20},
                {1, 190, 69},
                {1, 150, 20}
        };
        Matrix X = new Matrix(x);
        norm(X);
        Matrix Beta = findBeta(X, Y);
        System.out.println("beta(model):");
        Beta.print(5,3);
        Matrix Ym = X.times(Beta);
        System.out.println("real Y:");
        Y.print(3,3);
        System.out.println("assumed(module) Y:");
        Ym.print(3,3);
        accuracy(Y, Ym);
    }

    public static void accuracy (Matrix Y, Matrix Ym) {
        double sum = 0.0;
        int _r = Y.getRowDimension();
        for (int r = 0; r < _r; r++) {
            sum += Math.abs(Y.get(r, 0) - Ym.get(r, 0)) / Y.get(r, 0);
        }
        double acc = 100.0 -(100.0 / Y.getRowDimension() * sum);
        System.out.println("accuracy:  " + acc);
    }

    public static void norm(Matrix T) {
        int _c = T.getColumnDimension();
        int _r = T.getRowDimension();
        for (int c = 0; c < _c; c++) {
            double max = T.get(0, c);
            double min = max;
            for (int r = 1; r < _r; r++){
                if (T.get(r, c) > max)
                    max = T.get(r, c);
                if (T.get(r, c) < min)
                    min = T.get(r, c);
            }
            double range;
            if (max == min)// to avoid division by zero (happens when parameters are equal)
                range = max;// or min
            else
                range = max - min;
            for (int r = 0; r < _r; r++){
                T.set(r, c, T.get(r, c) / range);
            }
        }
    }

    public static Matrix findBeta(Matrix X, Matrix Y){
        Matrix Xt = X.transpose();
        Matrix XtX = Xt.times(X);
        Matrix _XtX = XtX.inverse();
        Matrix _XtXXt = _XtX.times(Xt);
        return _XtXXt.times(Y);
    }
}
