@rem = '--*-Perl-*--
@echo off
if "%OS%" == "Windows_NT" goto WinNT
perl -x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
:WinNT
perl -x -S %0 %*
if NOT "%COMSPEC%" == "%SystemRoot%\system32\cmd.exe" goto endofperl
if %errorlevel% == 9009 echo You do not have Perl in your PATH.
if errorlevel 1 goto script_failed_so_exit_with_non_zero_val 2>nul
goto endofperl
@rem ';
#!/usr/bin/perl
#line 15
use strict;

open(my $f,'<binaries.txt') || die "Failed to open binaries.txt:$!";


my @bins;
my @libs;

while(<$f>)
{
  s/[\x0d\x0a]//g;
  next unless length($_);
  next if /^#/;
  unless(/([el]):(.+?):([^:]+)(?::(.+))?/)
  {
    print STDERR "Invalid line in binaries.txt: '$_'";
    exit;
  }
  my $type=$1;
  my $name=$2;
  my $src=$3;
  my $deps=$4;
  if($type eq 'e')
  {
    push @bins,{binary=>$name,source=>$src,deps=>[split(/\s+/,$4)]};
  }elsif($type eq 'l')
  {
    push @libs,{name=>$name,source=>$src,dir=>$src};
  }
}

my @subdirs;

for my $bin(@bins)
{
  my $dir=$bin->{source};
  $dir=~s!/[^/]+$!/!;
  push @subdirs,$dir;
  $bin->{srclist}=[];
  opendir(my $d,$dir) || die "Failed to open current directory '$dir' for reading:$!";
  for my $file(readdir($d))
  {
    next unless $file=~/\.cpp$/;
    $file=$dir.$file;
    next if $file eq $bin->{source};
    push @{$bin->{srclist}},$file;
  }
  closedir($d);
}

for my $lib(@libs)
{
  my $dir=$lib->{dir};
  $dir.='/' unless $dir=~m!/$!;
  push @subdirs,$dir;
  $lib->{srclist}=[];
  opendir(my $d,$dir) || die "Failed to open current directory '$dir' for reading:$!";
  for my $file(readdir($d))
  {
    next unless $file=~/\.cpp$/;
    $file=$dir.$file;
    push @{$lib->{srclist}},$file;
  }
  closedir($d);
}

open(my $out,'>makefile.main') || die "Failed to open makefile.main for writing";

print $out "all: ".join(' ',map{'$(BUILDDIR)/bin/'.$_->{binary}.'$(BINEXT)'}@bins).' '.join(' ',map{'$(BUILDDIR)/lib/lib'.$_->{name}.'.a'}@libs)."\n\n";

print $out "dirs:\n";
print $out "\t\@mkpath lib\n";
for my $d(@subdirs)
{
  $d=~s!/$!!;
  print $out "\t\@mkpath \$(OBJDIR)/$d\n";
  print $out "\t\@mkpath \$(DEPSDIR)/$d\n";

}
for my $bin(@bins)
{
  my $dir=$bin->{binary};
  $dir=~s!/[^/]+$!/!;
  print $out "\t\@mkpath \$(BUILDDIR)/bin/$dir\n";
}
print $out "\t\@mkpath \$(BUILDDIR)/lib\n";
print $out "\n";

print $out "CXXFLAGS+= ".join(' ',map{"-I$_"}@subdirs)."\n\n";

my @allsrc;

for my $bin(@bins)
{
  print $out '$(BUILDDIR)/bin/'.$bin->{binary}.'$(BINEXT) : $(OBJDIR)/'.src2obj($bin->{source}).' '.
             join(' ',map{'$(OBJDIR)/'.src2obj($_)}@{$bin->{srclist}}).' '.
             join(' ',map{'$(BUILDDIR)/lib/lib'.$_.'.a'}@{$bin->{deps}})."\n";
  print $out "\t\@echo Linking $bin->{binary}\$(BINEXT)\n";
  print $out "\t\@\$(CXX) -o \$(BUILDDIR)/bin/$bin->{binary}\$(BINEXT) \$(OBJDIR)/".src2obj($bin->{source}).' '.
             join(' ',map{'$(OBJDIR)/'.src2obj($_)}@{$bin->{srclist}}).' '.
             " -L\$(BUILDDIR)/lib ".join(' ',map{'-l'.$_}@{$bin->{deps}}).' $(LDFLAGS) '.join(' ',map{'-l'.$_}@{$bin->{deps}})."\n";
  print $out "\t\@echo Linking $bin->{binary}\$(BINEXT) done\n";
  print $out "\n";
  push @allsrc,$bin->{source};
  push @allsrc,@{$bin->{srclist}};
}

for my $lib(@libs)
{
  print $out '$(BUILDDIR)/lib/lib'.$lib->{name}.'.a : '.join(' ',map{'$(OBJDIR)/'.src2obj($_)}@{$lib->{srclist}})."\n";
  print $out "\t\@echo Assembling $lib->{name}\n";
  print $out "\t\@\$(AR) -r \$(BUILDDIR)/lib/lib$lib->{name}.a ".join(' ',map{'$(OBJDIR)/'.src2obj($_)}@{$lib->{srclist}})."\n";
  print $out "\n";
  push @allsrc,@{$lib->{srclist}};
}

for my $f(@allsrc)
{
  my $fn=$f;
  $fn=~s/\.cpp$//;
  print $out "-include \$(DEPSDIR)/$fn.dep\n";
  print $out "\$(OBJDIR)/".src2obj($f).": $f\n";
  print $out "\t\@echo Compiling $f\n";
#  print $out "\t\@FN=$fn\n";
  my $dep;
  if($^O ne 'darwin')
  {
    $dep="-MT '\$(OBJDIR)/$fn.o'";
  }
  print $out "\t\@\$(CXX) \$(CXXFLAGS) -c -o \$(OBJDIR)/$fn.o"." -MMD $dep -MF \$(DEPSDIR)/$fn.depx $f\n";
  print $out "\t\@mv \$(DEPSDIR)/$fn.depx \$(DEPSDIR)/$fn.dep\n\n";
}

print $out "clean:\n";
for my $bin(@bins)
{
  print $out "\t\@rm -f \$(BUILDDIR)/bin/".$bin->{binary}."\$(BINEXT)\n";
}
for my $lib(@libs)
{
  print $out "\t\@rm -f \$(BUILDDIR)/lib/lib".$lib->{name}.".a\n";
}
for my $f(@allsrc)
{
  my $fn=$f;
  $fn=~s/\.cpp$//;
  print $out "\t\@rm -f \$(OBJDIR)/$fn.o\n";
  print $out "\t\@rm -f \$(DEPSDIR)/$fn.dep\n";
  print $out "\t\@rm -f \$(DEPSDIR)/$fn.depx\n";
}
print $out "\n";


sub src2obj{
  my $f=shift;
  $f=~s/\.cpp$/\.o/;
  return $f;
}

__END__
:endofperl
