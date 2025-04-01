package com.example.calculadora_engenharia;

import static androidx.constraintlayout.widget.ConstraintSet.VISIBLE;

import android.os.Bundle;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class MainActivity extends androidx.appcompat.app.AppCompatActivity {
    // Variáveis de Saídas
    private android.widget.TextView eaqucao_selecionada;
    private android.widget.TextView a_titulo;
    private android.widget.TextView a_unidade;
    private android.widget.TextView b_titulo;
    private android.widget.TextView b_unidade;
    private android.widget.TextView c_titulo;
    private android.widget.TextView c_unidade;
    private android.widget.TextView d_titulo;
    private android.widget.TextView d_unidade;
    private android.widget.TextView e_titulo;
    private android.widget.TextView e_unidade;
    private android.widget.TextView analise;
    private android.widget.TextView analise_unidade;

    // Variáveis de Entradas

    private android.widget.Spinner equacoes;

    private android.widget.EditText a;
    private android.widget.EditText b;
    private android.widget.EditText c;
    private android.widget.EditText d;
    private android.widget.EditText e;
    private android.widget.Button botao;
    private android.widget.RadioGroup unidade;
    private android.widget.ArrayAdapter arrayAdaptada;
    private Equacoes novaEquacao;

    // Médido de inicialização
    @Override
    protected void onCreate(android.os.Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        androidx.activity.EdgeToEdge.enable(this);
        setContentView(com.example.calculadora_engenharia.R.layout.activity_main);

        // Relacionando variáveis de Saída
        eaqucao_selecionada = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_eaqucao_selecionada);
        a_titulo = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_a_titulo);
        a_unidade = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_a_unidade);
        b_titulo = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_b_titulo);
        b_unidade = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_b_unidade);
        c_titulo = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_c_titulo);
        c_unidade = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_c_unidade);
        d_titulo = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_d_titulo);
        d_unidade = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_d_unidade);
        e_titulo = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_e_titulo);
        e_unidade = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_e_unidade);
        analise = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_analise);
        analise_unidade = (android.widget.TextView)findViewById(com.example.calculadora_engenharia.R.id.textview_analise_unidade);

        // Relacionando Variáveis de Entrada
        equacoes = (android.widget.Spinner)findViewById(com.example.calculadora_engenharia.R.id.spinner_equacao);
        a = (android.widget.EditText)findViewById(com.example.calculadora_engenharia.R.id.edittext_a);
        b = (android.widget.EditText)findViewById(com.example.calculadora_engenharia.R.id.edittext_b);
        c = (android.widget.EditText)findViewById(com.example.calculadora_engenharia.R.id.edittext_c);
        d = (android.widget.EditText)findViewById(com.example.calculadora_engenharia.R.id.edittext_d);
        e = (android.widget.EditText)findViewById(com.example.calculadora_engenharia.R.id.edittext_e);
        botao = (android.widget.Button)findViewById(com.example.calculadora_engenharia.R.id.button_calcular);
        unidade = (android.widget.RadioGroup)findViewById(com.example.calculadora_engenharia.R.id.RadioGroup_selecao);

        // Inicialização da Variável de Classe Externa
        novaEquacao = new com.example.calculadora_engenharia.Equacoes();

        // Analisando selações na tela
        android.widget.AdapterView.OnItemSelectedListener itemSelectedListener = new android.widget.AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(android.widget.AdapterView<?> adapterView, android.view.View view, int i, long l) {
                String string = ((android.widget.TextView)view).getText().toString();
                if(i==0){
                    eaqucao_selecionada.setText("ΔT = ΔL / (Lo * a)");
                    a_titulo.setText("ΔL");
                    a.setText("");
                    a_unidade.setText("m");
                    b_titulo.setText("Lo");
                    b.setText("");
                    b_unidade.setText("m");
                    c_titulo.setVisibility(c_titulo.VISIBLE);
                    c_titulo.setText("a");
                    c.setText("");
                    c_unidade.setVisibility(c_titulo.VISIBLE);
                    c.setVisibility(c_titulo.VISIBLE);
                    c_unidade.setText("°C");
                    d_titulo.setVisibility(d_titulo.INVISIBLE);
                    d.setVisibility(c_titulo.INVISIBLE);
                    d.setText("0");
                    d_unidade.setVisibility(d_unidade.INVISIBLE);
                    e_titulo.setVisibility(e_titulo.INVISIBLE);
                    e.setVisibility(c_titulo.INVISIBLE);
                    e.setText("0");
                    e_unidade.setVisibility(e_unidade.INVISIBLE);
                }
                if(i==1){
                    eaqucao_selecionada.setText("ΔT = Q / C");
                    a_titulo.setText("Q");
                    a.setText("");
                    a_unidade.setText("°C");
                    b_titulo.setText("C");
                    b.setText("");
                    b_unidade.setText("J/K");
                    c_titulo.setVisibility(c_titulo.INVISIBLE);
                    c.setVisibility(c_titulo.INVISIBLE);
                    c.setText("0");
                    c_unidade.setVisibility(c_unidade.INVISIBLE);
                    d_titulo.setVisibility(d_titulo.INVISIBLE);
                    d.setVisibility(c_titulo.INVISIBLE);
                    d.setText("0");
                    d_unidade.setVisibility(d_unidade.INVISIBLE);
                    e_titulo.setVisibility(e_titulo.INVISIBLE);
                    e.setVisibility(c_titulo.INVISIBLE);
                    e.setText("0");
                    e_unidade.setVisibility(e_unidade.INVISIBLE);
                }
                novaEquacao.setSelecao(i);
            }
            @Override
            public void onNothingSelected(android.widget.AdapterView<?> adapterView) {
            }
        };
        equacoes.setOnItemSelectedListener(itemSelectedListener);
    }
    // Seleção da unidade de medida
    public void onRadioButtonClick(android.view.View view){
        int id = unidade.getCheckedRadioButtonId();
        if (id == com.example.calculadora_engenharia.R.id.radio_1){
            novaEquacao.setUnidade(0);
            analise_unidade.setText("°C");
        } else if (id == com.example.calculadora_engenharia.R.id.radio_2) {
            novaEquacao.setUnidade(1);
            analise_unidade.setText("°F");
        } else if (id == com.example.calculadora_engenharia.R.id.radio_3) {
            novaEquacao.setUnidade(2);
            analise_unidade.setText("K");
        }
    }

    // Processo ao ser clicado no botão
    public void calcular_botao(android.view.View view) {

        // Guarda Seleções
        int SELECAO = novaEquacao.getSelecao();
        int UNIDADE = novaEquacao.getUnidade();
        // Tenta converter números digitado para Float
        try {
            // Parse input values from the text fields
            int A = Integer.parseInt(String.valueOf(a.getText()));
            int B = Integer.parseInt(String.valueOf(b.getText()));
            int C = Integer.parseInt(String.valueOf(c.getText()));
            int D = Integer.parseInt(String.valueOf(d.getText()));
            int E = Integer.parseInt(String.valueOf(e.getText()));

            // Envia informacoes para obter resultado da equacao selecionada
            String r = String.valueOf(novaEquacao.Operacoes(SELECAO, UNIDADE, A, B, C, D, E));
            // Exibe resultado
            if (r == null) {
                analise.setText("Operação não retornou resultado!");
            } else {
                analise.setText(r);
            }
        }catch (NumberFormatException e) {
            // Resultado em caso de números inválidos
            analise.setText("Invalido");
        } catch (Exception e) {
            // Qualquer outro erro
            analise.setText("Erro");
        }
    }
}