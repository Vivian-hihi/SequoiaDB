package com.sequoiadb.wordConvertor;

import java.io.File;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.GnuParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.log4j.Logger;

public class ConvertHelper {
    private static Logger logger = Logger.getLogger(ConvertHelper.class);
    private String input;
    private String output;
    private Boolean hasTableOfContent = false;
    public String getInput() {
        return input;
    }

    public String getOutput() {
        return output;
    }

    public Boolean getHasTableOfContent() {
        return hasTableOfContent;
    }

    public static WordConvertor createConvertor(){
        return null;
    }
    
    public int parseArgs(String args[]){
        Options opt = new Options();
        opt.addOption("h", "help", false, "help");
        opt.addOption("i", "input", true, "input html file");
        opt.addOption("o", "output", true, "output word file");
        opt.addOption("t", "table", false, "add table of content");
        CommandLineParser parser = new GnuParser();
        CommandLine cl = null;
        try{
            cl = parser.parse(opt, args);
        }
        catch(ParseException e){
            new HelpFormatter().printHelp("WordConvertor", opt);
            return -1;
        }
        
        if (cl.hasOption("h")){
            new HelpFormatter().printHelp("WordConvertor", opt);
            return -2;
        }
        
        if (!cl.hasOption("i")){
            System.err.println("input html file must be specified!");
            return -1;
        }
        
        if (!cl.hasOption("o")){
            System.err.println("input html file must be specified!");
            return -1;
        }
        
        input = cl.getOptionValue("i");
        File f = new File(input);
        input = f.getAbsolutePath();
        logger.info("input file=" + input);
        
        output = cl.getOptionValue("o");
        f = new File(output);
        output = f.getAbsolutePath();
        logger.info("output file=" + output);
        
        if (cl.hasOption("t")){
            hasTableOfContent = true;
        }
        
        return 0;
    }
}
