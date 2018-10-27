package CW_Implicit;

public class Stats {
    private double meanAbsoluteError;
    private double maxAbsoluteError;
    private double meanRelativeError;
    private double maxRelativeError;

    private double [][] real;
    private double [][] found;

    Stats(double [][] real, double [][] found) {
        this.real = real;
        this.found = found;
        meanAE();
        maxAE();
        meanRE();
        maxRE();
    }

    public void showStats() {
        System.out.println("Mean Absol er:  " + this.meanAbsoluteError);
        System.out.println("Max Absol er:  " + this.maxAbsoluteError);
        System.out.println("Mean Rel er:  " + this.meanRelativeError);
        System.out.println("Max Rel er:  " + this.maxRelativeError);
    }

    private void meanAE() {
        double c = 0.0;
        double sum = 0.0;
        for (int i = 1; i < real.length; i++) {
            for (int j = 1;j < real[i].length - 1; j++) {
                if (real[i][j] != 0.0 && found[i][j] != 0.0)
                    sum += Math.abs(real[i][j] - found[i][j]) / real[i][j];
                c += 1.0;
            }
        }
        this.meanAbsoluteError = sum / c;
    }

    private void maxAE() {
        double max = 0.0;
        double cur;
        for (int i = 1; i < real.length; i++) {
            for (int j = 1;j < real[i].length - 1; j++) {
                if (real[i][j] != 0.0 && found[i][j] != 0.0) {
                    cur = Math.abs(real[i][j] - found[i][j]) / real[i][j];
                    if (cur > max)
                        max = cur;
                }
            }
        }
        this.maxAbsoluteError = max;
    }

    private void meanRE() {
        double sum = 0.0;
        double c = 0.0;
        for (int i = 1; i < real.length; i++) {
            for (int j = 1;j < real[i].length - 1; j++) {
                if (real[i][j] != 0.0 && found[i][j] != 0.0)
                    sum += Math.abs((found[i][j] - real[i][j]) / real[i][j]);
                c += 1.0;
            }
        }
        this.meanRelativeError = sum / c;
    }

    private void maxRE() {
        double max = 0.0;
        for (int i = 1; i < real.length; i++) {
            for (int j = 1;j < real[i].length - 1; j++) {
                if (real[i][j] != 0.0 && found[i][j] != 0.0) {
                    double cur = Math.abs((found[i][j] - real[i][j]) / real[i][j]);
                    if (cur > max)
                        max = cur;
                }
            }
        }
        this.maxRelativeError = max;
    }

}
