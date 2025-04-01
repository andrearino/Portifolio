package com.example.calculadora_engenharia;

public class Equacoes {

    private int selecao;
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;
    private int unidade;
    private boolean flag;

    // Methods to set values
    public void setSelecao(int value) {
        if (value < 0) {
            throw new IllegalArgumentException("Cannot be negative.");
        }
        this.selecao = value;
    }

    public int getSelecao() {
        return this.selecao;
    }

    public void setA(int value) {
        if (value < 0) {
            throw new IllegalArgumentException("Cannot be negative.");
        }
        this.a = value;
    }

    public int getA() {
        return this.a;
    }

    public void setB(int value) {
        if (value < 0) {
            throw new IllegalArgumentException("Cannot be negative.");
        }
        this.b = value;
    }

    public int getB() {
        return this.b;
    }

    public void setC(int value) {
        if (value < 0) {
            throw new IllegalArgumentException("Cannot be negative.");
        }
        this.c = value;
    }

    public int getC() {
        return this.c;
    }

    public void setD(int value) {
        if (value < 0) {
            throw new IllegalArgumentException("Cannot be negative.");
        }
        this.d = value;
    }

    public int getD() {
        return this.d;
    }

    public void setE(int value) {
        if (value < 0) {
            throw new IllegalArgumentException("Cannot be negative.");
        }
        this.e = value;
    }

    public int getE() {
        return this.e;
    }

    public void setUnidade(int value) {
        if (value < 0) {
            throw new IllegalArgumentException("Cannot be negative.");
        }
        this.unidade = value;
    }

    public int getUnidade() {
        return this.unidade;
    }

    public void setFLAG(boolean value) {
        if (value != Boolean.TRUE && value != Boolean.FALSE) {
            throw new IllegalArgumentException("Cannot be negative.");
        }
        this.flag = value;
    }

    public boolean getFLAG() {
        return this.flag;
    }

    public float Operacoes(int SELECAO, int UNIDADE, int A, int B, int C, int D, int E) {
        this.selecao = SELECAO;
        this.unidade = UNIDADE;
        this.a = A;
        this.b = B;
        this.c = C;
        this.d = D;
        this.e = E;

        int dT = 0;
        int flag = 0;

        // Falha na seleção
        if (this.selecao != 0 && this.selecao != 1) {
            throw new IllegalArgumentException("Seleçao must be 0 (Dilataçao) or 1 (Capacidade).");
        }

        // Equação de Dilataçao Termica: dT = dL / (Lo * a)
        if (this.selecao == 0) {
            if (this.b == 0 || this.c == 0) {
                throw new ArithmeticException("Coefficient 'Lo' or 'a' cannot be zero.");
            } else {
                if (unidade == 0) {
                    dT = this.a / (this.b * this.c);
                    int dT1 = dT;
                    return dT1;
                }
                if (unidade == 1) {
                    dT = this.a / (this.b * this.c);
                    dT = ((9 * dT) / 5)  + 32;
                    int dT1 = dT;
                    return dT1;
                }
                if (unidade == 2) {
                    dT = this.a / (this.b * this.c);
                    dT = (int) (dT + 273.15);
                    int dT1 = dT;
                    return dT1;
                }
            }
        }

        // Equação de Capacidade Termica: dT = Q / C
        if (this.selecao == 1) {
            if (this.b == 0) {
                throw new ArithmeticException("Coefficient 'C' cannot be zero.");
            } else {
                if (unidade == 0) {
                    dT = this.a / this.b;
                    int dT1 = dT;
                    return dT1;
                }
                if (unidade == 1) {
                    dT = this.a / this.b;
                    dT = ((9 * dT) / 5)  + 32;
                    int dT1 = dT;
                    return dT1;
                }
                if (unidade == 2) {
                    dT = this.a / this.b;
                    dT = (int) (dT + 273.15);
                    int dT1 = dT;
                    return dT1;
                }
            }
        }
        int dT1 = 0;
        return dT1;
    }
}
