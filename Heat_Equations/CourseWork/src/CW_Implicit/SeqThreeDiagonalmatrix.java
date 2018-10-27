package CW_Implicit;

public class SeqThreeDiagonalmatrix {
    double[] mainDiagonal;
    double[] upperDiagonal;
    double[] lowerDiagonal;

    public SeqThreeDiagonalmatrix(int size) {
        mainDiagonal = new double[size];
        upperDiagonal = new double[size - 1];
        lowerDiagonal = new double[size - 1];
    }

    public void setLowerDiagonalElement(int index, double value) {
        lowerDiagonal[index] = value;
    }

    public void setUpperDiagonalElement(int index, double value) {
        upperDiagonal[index] = value;
    }

    public void setMainDiagonalElement(int index, double value) {
        mainDiagonal[index] = value;
    }

    public void Solve(double[] freeTerms, double[] result) {
        for (int i = 0; i < result.length - 1; ++i) {
            double alpha = - (lowerDiagonal[i] / mainDiagonal[i]);
            lowerDiagonal[i] = 0.0;
            mainDiagonal[i + 1] += alpha * upperDiagonal[i];
            freeTerms[i + 1] += alpha * freeTerms[i];
        }

        result[result.length - 1] = freeTerms[freeTerms.length - 1] / mainDiagonal[mainDiagonal.length - 1];

        for (int i = result.length - 2; i >= 0; --i) {
            result[i] = (freeTerms[i] - upperDiagonal[i] * result[i + 1]) / mainDiagonal[i];
        }
    }
}
